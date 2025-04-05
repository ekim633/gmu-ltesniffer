#include "include/ArgManager.h"
#include "include/LTESniffer_Core.h"

#include "falcon/common/Version.h"
#include "falcon/common/SignalManager.h"

#include <iostream>
#include <memory>
#include <cstdlib>
#include <unistd.h>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <SDL2/SDL.h>
#include <uhd/usrp/multi_usrp.hpp>

#include "imgui_internal.h"
#include "build/debug/srsRAN-src/srsenb/hdr/stack/mac/sched_phy_ch/sched_phy_resource.h"
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL2/SDL_opengles2.h>
#else
#include <SDL2/SDL_opengl.h>
#endif

std::vector<std::vector<int>> resourceBlockHistory;
const int NUM_RESOURCE_BLOCKS = 50;
const int DISPLAY_HISTORY = 10;

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

Args global_args;

uint32_t global_target_rnti = 0;
std::mutex global_target_rnti_mutex;

char global_target_imsi[32];
std::mutex global_target_imsi_mutex;

uint32_t global_rb_allocation_indices[MAX_NOF_PRBS] = {};
uint32_t global_rb_allocation_indices_count = 0;
bool global_rb_allocation_indices_changed = false;
std::mutex global_rb_allocation_indices_mutex;

uint32_t global_ui_nof_prb = 0;
bool global_ui_nof_prb_changed = false;
std::mutex global_ui_nof_prb_mutex;

bool global_sniffer_ready = false;
std::mutex global_sniffer_ready_mutex;

struct Display_Rb {
    double frequency;
    bool allocated;
};

static bool is_point_in_rectangle(ImVec2 point, ImVec2 rect_min, ImVec2 rect_max);
static int32_t binary_search(uint32_t *data, uint32_t length, uint32_t value);

int run() {
  cout << endl;
  cout << "LTESniffer" << endl;
  cout << endl;

  //attach signal handlers (for CTRL+C)
  SignalGate& signalGate(SignalGate::getInstance());
  signalGate.init();

  LTESniffer_Core SnifferCore(global_args);
  signalGate.attach(SnifferCore);

  bool success = SnifferCore.run();

  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

// Main code
int main(int argc, char** argv) {
    ArgManager::parseArgs(global_args, argc, argv);

#if 0
    uhd::usrp::multi_usrp::sptr jammer_usrp = uhd::usrp::multi_usrp::make("type=b200");
    // jammer_usrp->set_tx_gain(0.0f);
    uhd::tx_streamer::sptr jammer_tx_stream = jammer_usrp->get_tx_stream(uhd::stream_args_t{"sc8", "sc8"});
#else
    uhd::usrp::multi_usrp::sptr jammer_usrp;
    uhd::tx_streamer::sptr jammer_tx_stream;
#endif

    std::thread lte_sniffer_thread{run};

    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL2/SDL_HINT_IME_SHOW_UI, "1");
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Jammer Controls", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 500, 200, window_flags);
    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    ImU32 im_blue = IM_COL32(61, 124, 255, 255);
    ImU32 im_red = IM_COL32(255, 0, 0, 255);
    ImU32 im_green = IM_COL32(0, 168, 7, 255);
    ImU32 im_purple = IM_COL32(109, 59, 140, 255);
    ImU32 im_white = IM_COL32(255, 255, 255, 255);

    float prb_bandwidth = 180e3;

    Display_Rb display_rbs[MAX_NOF_PRBS];
    bool display_rbs_dirty = true;

    uint32_t last_rb_allocation_update_ticks = SDL_GetTicks();

    bool ready = false;

    int32_t jammed_prb_index = -1;
    int8_t jam_data[512];
    uhd::tx_streamer::buffs_type jam_buffers{jam_data};
    uhd::tx_metadata_t jam_metadata{};

    bool jamming_enabled = false;

    // Main loop
    bool done = false;
    while (!done) {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
        {
            SDL_Delay(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        if (ready) {
            global_ui_nof_prb_mutex.lock();
            uint32_t nof_prb = global_ui_nof_prb;
            if (global_ui_nof_prb_changed) {
                display_rbs_dirty = true;
                global_ui_nof_prb_changed = false;
            }
            global_ui_nof_prb_mutex.unlock();


            global_rb_allocation_indices_mutex.lock();
            if (global_rb_allocation_indices_changed) {
                last_rb_allocation_update_ticks = SDL_GetTicks();
                display_rbs_dirty = true;
                if (global_args.enable_jamming) {
                    if (global_rb_allocation_indices_count > 0) {
                        jammed_prb_index = (int32_t)global_rb_allocation_indices[0];
                        assert(jammed_prb_index < nof_prb);
                        jammer_usrp->set_tx_freq(display_rbs[jammed_prb_index].frequency);
                    } else {
                        printf("Tried to choose new PRB to jam, but there were none\n");
                        jammed_prb_index = -1;
                    }
                }
                global_rb_allocation_indices_changed = false;
            }

            if (display_rbs_dirty) {
                for (uint32_t i = 0; i < nof_prb; i++) {
                    display_rbs[i].frequency = global_args.rf_freq - (((double)nof_prb * prb_bandwidth) / 2.0) + ((double)i * prb_bandwidth) + (prb_bandwidth / 2.0);
                    display_rbs[i].allocated = (binary_search(global_rb_allocation_indices, global_rb_allocation_indices_count, i) >= 0);
                }

                display_rbs_dirty = false;
            }
            global_rb_allocation_indices_mutex.unlock();

            if (jammed_prb_index >= 0 && jamming_enabled) {
                assert(jammed_prb_index < nof_prb);
                jammer_tx_stream->send(jam_buffers, ARRAY_SIZE(jam_data), jam_metadata);
            }

            // TODO: it would be nice to show the other (non-target) RBs in the RB visualization grid


            char imsi_buffer[16] = {};
            ImGui::SetNextWindowSize(ImVec2(1000, 1200));
            ImGui::Begin("Info");
            if (global_args.enable_jamming) {
                ImGui::Checkbox("Jamming Enabled", &jamming_enabled);
                ImGui::Text("Jammed PRB %d", jammed_prb_index);
            }

            ImGui::SetKeyboardFocusHere();
            // ImGui::Text("Current IMSI %s", global_target_imsi);
            // if (ImGui::InputText("IMSI", imsi_buffer, sizeof(imsi_buffer), ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue)) {
            //     printf("New target IMSI: %s\n", imsi_buffer);
            //     global_target_imsi_mutex.lock();
            //     strncpy(global_target_imsi, imsi_buffer, sizeof(global_target_imsi));
            //     global_target_imsi_mutex.unlock();
            // }
            ImGui::Text("Current RNTI %d", global_target_rnti);
            char rnti_buffer[16] = {};
            if (ImGui::InputText("RNTI", rnti_buffer, sizeof(imsi_buffer), ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_EnterReturnsTrue)) {
                printf("New target RNTI: %s\n", rnti_buffer);
                global_target_rnti_mutex.lock();
                char *end;
                long rnti = strtol(rnti_buffer, &end, 10);

                if (*end == 0) {
                    global_target_rnti = rnti;
                }
                global_target_rnti_mutex.unlock();
            }

            ImGui::Text("Center Frequency DL: %.03f MHz", global_args.rf_freq / 1e6);
            ImGui::Text("# RBs: %u", nof_prb);
            ImGui::Text("# RBs allocated: %u", global_rb_allocation_indices_count);
            ImGui::Text("Seconds since last RB allocation: %.02f", float(SDL_GetTicks() - last_rb_allocation_update_ticks) / 1000.0f);

            ImDrawList *draw_list = ImGui::GetWindowDrawList();
            ImVec2 start = ImGui::GetCursorScreenPos();
            ImVec2 mouse_position = ImGui::GetMousePos();

            ImVec2 data_min = {start.x, start.y};
            int32_t spacing_x = 1;
            int32_t spacing_y = 1;
            float outline_thickness = 4;
            float cell_width = 10;
            float cell_height = 10;
            //it used to be 20
            uint32_t max_columns = 1;
            float text_margin = 3;
            float section_spacing = 10;


            int32_t real_spacing_x = spacing_x;
            int32_t real_spacing_y = spacing_y;
            if (outline_thickness > 0) {
                real_spacing_x += (outline_thickness / 2);
                real_spacing_y += (outline_thickness / 2);
            }

            float bottom_y = data_min.y + float(nof_prb / max_columns) * ((int32_t)cell_height + real_spacing_y) + cell_height;
            int32_t selected_prb = -1;
            for (uint32_t i = 0; i < nof_prb; ++i) {
                ImVec2 cell_min = {};
                cell_min.x = data_min.x + float(int32_t(i % max_columns) * ((int32_t)cell_width + real_spacing_x));
                cell_min.y = data_min.y + float(int32_t(i / max_columns) * ((int32_t)cell_height + real_spacing_y));

                ImVec2 cell_max = {};
                cell_max.x = cell_min.x + (float)cell_width;
                cell_max.y = cell_min.y + (float)cell_height;

                ImU32 color = im_red;
                if (display_rbs[i].allocated) {
                    color = im_green;
                }
                if (jammed_prb_index >= 0 && i == jammed_prb_index) {
                    color = im_purple;
                }

                draw_list->AddRectFilled(cell_min, cell_max, color);

                if (is_point_in_rectangle(mouse_position, cell_min, cell_max)) {
                    selected_prb = (int32_t)i;
                    ImVec2 outline_min = {cell_min.x - ((float)outline_thickness / 2.0f), cell_min.y - ((float)outline_thickness / 2.0f)};
                    ImVec2 outline_max = {outline_min.x + (float)((int32_t)cell_width + outline_thickness), outline_min.y + (float)((int32_t)cell_height + outline_thickness)};
                    draw_list->AddRect(outline_min, outline_max, im_blue, 0, 0, (float)abs(outline_thickness));
                }
            }

            if (selected_prb >= 0) {
                ImFont * font = ImGui::GetFont();

                ImVec2 text_box_min = {start.x, bottom_y + section_spacing};
                ImVec2 text_start = {
                    text_box_min.x + text_margin,
                    text_box_min.y + text_margin
                };

                struct Message {
                    char text[64];
                    ImVec2 dimensions;
                };
                Message messages[8] = {};
                uint32_t message_count = 0;
                float widest_message_width = 0;
                float cumulative_text_height = 0;


#define NEW_MESSAGE(msg, ...) \
{ snprintf(messages[message_count].text, ARRAY_SIZE(messages[message_count].text), msg __VA_OPT__(,) __VA_ARGS__); \
messages[message_count].dimensions = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.0f, messages[message_count].text); \
if (messages[message_count].dimensions.x > widest_message_width) {widest_message_width = messages[message_count].dimensions.x;} \
++message_count; cumulative_text_height += messages[message_count].dimensions.y; } (void)0

                NEW_MESSAGE("RB: %d",          selected_prb);
                NEW_MESSAGE("Freq: %.02f MHz", display_rbs[selected_prb].frequency / 1e6);
                NEW_MESSAGE("Allocated: %s",   display_rbs[selected_prb].allocated ? "true" : "false");

#undef NEW_MESSAGE

                ImVec2 text_box_max = {text_box_min.x + widest_message_width + 2.0f * (float)text_margin,
                    text_box_min.y + cumulative_text_height + 2.0f * (float)text_margin};

                // draw_list->AddRectFilled(text_box_min, text_box_max, im_purple);
                float cumulative_text_y = text_start.y;
                for (uint32_t i = 0; i < message_count; ++i) {
                    ImVec2 position = {text_start.x, cumulative_text_y};
                    draw_list->AddText(position, im_white, messages[i].text);
                    cumulative_text_y += messages[i].dimensions.y;
                }
            }
            ImGui::End();
        } else {
            ImGui::Text("Loading...");
            global_sniffer_ready_mutex.lock();
            ready = global_sniffer_ready;
            global_sniffer_ready_mutex.unlock();
            if (ready && global_args.enable_jamming) {
                jammer_usrp = uhd::usrp::multi_usrp::make("type=b200");
                // tune into the frequency of the target
                //jammer_usrp->set_tx_freq(uhd::tune_request_t(2.45e9));
                // jammer_usrp->set_tx_gain(0.0f);

                jammer_tx_stream = jammer_usrp->get_tx_stream(uhd::stream_args_t{"sc8", "sc8"});
            }
            SDL_Delay(10);
        }

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

static bool is_point_in_rectangle(ImVec2 point, ImVec2 rect_min, ImVec2 rect_max) {
    return point.x >= rect_min.x
    && point.x <= rect_max.x
    && point.y >= rect_min.y
    && point.y <= rect_max.y;
}

static int32_t binary_search(uint32_t *data, uint32_t length, uint32_t value) {
    if (length == 0) {
        return -1;
    }

    int32_t low = 0;
    int32_t high = (int32_t)length;

    while (low <= high) {
        int32_t middle = low + (high - low) / 2;

        // If x greater, ignore left half
        if (data[middle] < value) {
            low = middle + 1;
        } else if (data[middle] > value) {
            // If x is smaller, ignore right half
            high = middle - 1;
        } else {
            // data[middle] == value
            // Check if x is present at mid
            return (int32_t)middle;
        }
    }

    // If we reach here, then element was not present
    return -1;
}
