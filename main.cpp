#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include "camera/camera.h"
#include "materials/materials.h"
#include "scene/scene_loader.h"
#include "core/tile_renderer.h"
#include "core/post_process.h"

using namespace rt;

int main(int argc, char* argv[]) {
    std::string pp_mode;
    bool spiral = false;
    std::vector<std::string> positional;

    for (int i = 1; i < argc; ++i) {
        std::string_view arg(argv[i]);
        if (arg.starts_with("--pp="))
            pp_mode = std::string(arg.substr(5));
        else if (arg == "--spiral")
            spiral = true;
        else
            positional.push_back(std::string(arg));
    }

    auto is_number = [](const std::string& s) {
        try { std::stod(s); return true; } catch (...) { return false; }
    };

    double      multiplier = 1.0;
    int         samples    = 64;
    std::string scene_path = "scenes/scene_dof.txt";

    if (!positional.empty() && is_number(positional[0])) {
        multiplier = std::stod(positional[0]);
        if (positional.size() > 1 && is_number(positional[1]))
            samples = std::stoi(positional[1]);
        if (positional.size() > 2)
            scene_path = positional[2];
    } else if (!positional.empty()) {
        scene_path = positional[0];
    }

    const int width        = (int)(400 * multiplier);
    const int height       = (int)(225 * multiplier);
    const double aspect_ratio = (double)width / height;

    Camera camera;
    std::map<std::string, Material> materials = build_materials();
    Scene scene = load_scene(scene_path, materials, camera, aspect_ratio);
    scene.build_bvh();

    RenderOutput out = render_tiles_hdr(scene, camera, width, height, samples, 16, spiral);
    std::cerr << "\n";

    PostPipeline pipeline;
    if (pp_mode == "tonemapping") {
        pipeline
            .add(tone_map_aces())
            .add(gamma_correct(2.2));
    } else if (pp_mode == "bloom") {
        pipeline
            .add(bloom_pass())
            .add(tone_map_aces())
            .add(gamma_correct(2.2));
    } else if (pp_mode == "ssao") {
        pipeline
            .add(ssao_pass(std::move(out.depth), std::move(out.normals), std::move(out.positions)))
            .add(tone_map_aces())
            .add(gamma_correct(2.2));
    } else if (pp_mode == "all") {
        pipeline
            .add(ssao_pass(std::move(out.depth), std::move(out.normals), std::move(out.positions)))
            .add(bloom_pass())
            .add(tone_map_aces())
            .add(gamma_correct(2.2));
    } else {
        // Mode brut : pas de tone mapping, mais gamma minimal pour éviter une image trop sombre.
        pipeline.add(gamma_correct(2.2));
    }
    pipeline.apply(out.color);

    std::cout << "P3\n" << width << " " << height << "\n255\n";
    for (int i = height - 1; i >= 0; i--) {
        for (int j = 0; j < width; j++) {
            const PixelHDR& p = out.color.at(j, i);
            int r = (int)(std::clamp(p.r, 0.0, 1.0) * 255);
            int g = (int)(std::clamp(p.g, 0.0, 1.0) * 255);
            int b = (int)(std::clamp(p.b, 0.0, 1.0) * 255);
            std::cout << r << " " << g << " " << b << " ";
        }
        std::cout << "\n";
    }
    return 0;
}
