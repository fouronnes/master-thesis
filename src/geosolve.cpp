#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <memory>
#include <string>
#include <map>
#include <functional>

#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/memory.hpp>

#include "ceres/ceres.h"

#include "project.h"
#include "model0.h"

using std::tuple;
using std::make_tuple;
using std::vector;
using std::array;
using std::string;
using std::shared_ptr;
using namespace std::placeholders;

Project base_model0_project() {
    Project project;

    project.data_set = std::shared_ptr<DataSet>(new DataSet());
    project.data_set->filenames.push_back(string("alinta-stockpile/DSC_5522.JPG"));
    project.data_set->filenames.push_back(string("alinta-stockpile/DSC_5521.JPG"));
    project.data_set->rows = 2832;
    project.data_set->cols = 4256;

    project.features = std::shared_ptr<FeaturesGraph>(new FeaturesGraph());
    project.features->data_set = project.data_set;
    project.features->number_of_matches = 10;
    project.features->add_edge(0, 1);

    std::shared_ptr<Model0> model(new Model0());

    // Initialize initial solution from parent model
    // (for now hard coded left-right images)
    model->features = project.features;
    model->internal = {48.3355e-3, 0.0093e-3, -0.0276e-3};
    model->pixel_size = 0.0085e-3;
    Model0::solution init;
    init.cameras.push_back({0, 0, 269, 0, 0, 0});
    init.cameras.push_back({0, 0, 269, 0, 0, 0});
    model->solutions.push_back(init);

    project.model = model;

    return project;
}

void add_model_terrain(Project& project) {
}

void base_model0(const string&, const string& project_dir) {
    Project project = base_model0_project();
    project.to_file(project_dir + "/project.json");
}

void base_model0_200(const string&, const string& project_dir) {
    Project project = base_model0_project();
    project.features->number_of_matches = 200;
    project.to_file(project_dir + "/project.json");
}

void base_model0_scale(const string&, const string& project_dir, double compute_scale) {
    Project project = base_model0_project();
    project.features->compute_scale = compute_scale;
    project.to_file(project_dir + "/project.json");
}

void load_test(const string&, const string& project_dir) {
    Project project = Project::from_file(project_dir + "/project.json");
    project.to_file(project_dir + "/loadtest-output.json");
}

void features(const string& data_dir, const string& project_dir) {
    string project_filename = project_dir + "/project.json";
    Project project = Project::from_file(project_filename);
    std::cout << "Computing features" << std::endl;
    project.features->compute(data_dir);
    project.to_file(project_filename);
}

void solve(const string&, const string& project_dir) {
    string project_filename = project_dir + "/project.json";
    std::cout << "Solving..." << std::endl;
    Project project = Project::from_file(project_filename);
    // Verify features have been computed
    if (!project.model->features || project.model->features->edges.size() == 0 || project.model->features->computed == false) {
        throw std::runtime_error("Attempting to solve model but no observations are available");
    }

    project.model->solve();
    project.to_file(project_filename);
}

int main(int argc, char* argv[]) {
    google::InitGoogleLogging(argv[0]);

    if (argc < 4) {
        std::cerr << "Usage: ./geosolve <data_dir> <project_dir> command" << std::endl;
        return -1;
    }

    string data_dir(argv[1]);
    string project_dir(argv[2]);
    string command(argv[3]);

    std::map<string, std::function<void (const string&, const string&)>> commands {
        {"base_model0", base_model0},
        {"base_model0_200", base_model0_200},
        {"base_model0_half", std::bind(base_model0_scale, _1, _2, 0.5)},
        {"base_model0_quarter", std::bind(base_model0_scale, _1, _2, 0.25)},
        {"loadtest", load_test},
        {"features", features},
        {"solve", solve}
    };

    try {
        commands.at(command)(data_dir, project_dir);
    } catch (std::out_of_range& err) {
        std::cerr << "Invalid command: " << command << std::endl;
        return -1;
    }
}
