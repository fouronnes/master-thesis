#ifndef PROJECT_H
#define PROJECT_H

#include <memory>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/memory.hpp>
#include "data_set.h"
#include "image_features.h"
#include "model.h"

// Overload std::array for JSON to use []
namespace cereal {
    template <std::size_t N>
    void save(cereal::JSONOutputArchive& archive, std::array<double, N> const& list) {
        archive(cereal::make_size_tag(static_cast<cereal::size_type>(list.size())));
        for (auto && v : list) {
            archive(v);
        }
    }

    template <std::size_t N>
    void load(cereal::JSONInputArchive& archive, std::array<double, N>& list) {
        cereal::size_type size;
        archive(cereal::make_size_tag(size));
        if (static_cast<std::size_t>(size) != N) {
            throw std::runtime_error("Error loading std::array from JSON: incorrect size.");
        }
        for (auto && v : list) {
            archive(v);
        }
    }
}

struct Project {
    std::shared_ptr<DataSet> data_set;
    std::shared_ptr<FeaturesGraph> features;
    std::vector<std::shared_ptr<Model>> models;

    static Project from_file(const std::string& filename) {
        Project p;
        std::ifstream ifs(filename);
        if (!ifs.good()) {
            throw std::runtime_error("Can't open " + filename);
        }
        cereal::JSONInputArchive ar(ifs);
        p.serialize(ar);
        return p;
    }

    void to_file(const std::string& filename) {
        std::ofstream ofs(filename);
        cereal::JSONOutputArchive ar(ofs);
        this->serialize(ar);
    }

    template <class Archive>
    void serialize(Archive& ar) {
        ar(cereal::make_nvp("data_set", data_set),
           cereal::make_nvp("features", features),
           cereal::make_nvp("models", models));
    }
};

#endif
