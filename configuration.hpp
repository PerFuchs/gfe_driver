/**
 * Copyright (C) 2019 Dean De Leo, email: dleo[at]cwi.nl
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <cinttypes>
#include <iostream>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "common/error.hpp"

class Configuration; // forward declaration
namespace common { class Database; } // forward declaration
namespace library { class Interface; } // forward declaration

// Singleton interface
Configuration& configuration(); // retrieve the current singleton (client, server or standalone)

// Generic configuration error
DEFINE_EXCEPTION(ConfigurationError);

// Print the given message to the standard output
extern std::mutex _log_mutex;
#define LOG( msg ) { std::scoped_lock lock(_log_mutex); std::cout << msg << /* flush immediately */ std::endl; }

// Type of counter for the number of threads
enum ThreadsType { THREADS_READ, THREADS_WRITE, THREADS_TOTAL };

/**
 *Global configuration for the driver.
 * - Initialise (only one time) the singleton instance through Configuration::initialise(int argc, char* argv[])
 * - Access the singleton instance through the function ::configuration();
 * The class is not thread safe.
 */
class Configuration {
    // remove the copy ctors
    Configuration(const Configuration& ) = delete;
    Configuration& operator=(const Configuration& ) = delete;

    // properties
    uint64_t m_build_frequency { 5 * 60 * 1000 }; // in the aging experiment, the amount of time that must pass before each invocation to #build(), in milliseconds
    double m_coeff_aging { 0.0 }; // coefficient for the additional updates to perform
    common::Database* m_database { nullptr }; // handle to the database
    std::string m_database_path { "" }; // the path where to store the results
    double m_ef_vertices = 1; // expansion factor for the vertices in the graph
    double m_ef_edges = 1;  // expansion factor for the edges in the graph
    bool m_graph_directed = true; // whether the graph is undirected or directed
    std::string m_library_name; // the library to test
    double m_max_weight { 1.0 }; // the maximum weight that can be assigned when reading non weighted graphs
    uint64_t m_num_repetitions { 5 }; // when applicable, how many times the same experiment should be repeated
    int m_num_threads_read { 1 }; // number of threads to use for the read operations
    int m_num_threads_write { 1 }; // number of threads to use for the write (insert/update/delete) operations
    std::string m_path_graph_to_load; // the file must be accessible to the server
    uint64_t m_seed = 5051789ull; // random seed, used in various places in the experiments
    uint64_t m_timeout_seconds { 3600 }; // max time to complete an operation, in seconds (0 => indefinite)
    std::string m_update_log; // aging experiment through the log file
    std::unique_ptr<library::Interface> (*m_library_factory)(bool directed) {nullptr} ; // function to retrieve an instance of the library `m_library_name'
    bool m_validate_output = false; // whether to validate the execution results of the Graphalytics algorithms

    void set_build_frequency(uint64_t millisecs);
    void set_coeff_aging(double value); // Set the coefficient for `aging', i.e. how many updates (insertions/deletions) to perform w.r.t. to the size of the loaded graph
    void set_ef_vertices(double value);
    void set_ef_edges(double value);
    void set_num_repetitions(uint64_t value); // Set how many times to repeat the Graphalytics suite of algorithms
    void set_num_thread_read(int value); // Set the number of threads to use in the read operations.
    void set_num_thread_write(int value); // Set the number of threads to use in the write operations.
    void set_timeout(uint64_t seconds); // Set the timeout property
    void set_graph(const std::string& graph); // Set the graph to load and run the experiments

    // Set the path to the database
    void set_database_path(const std::string& path){ m_database_path = path; }

    // The max weight that can be assigned by graph readers when parsing a non weighted graph
    void set_max_weight(double value);

    // Set the property seed
    void set_seed(uint64_t value){ m_seed = value; }
public:
    // Default configuration
    Configuration();

    // Destructor
    ~Configuration();

    // Initialise the configuration with the arguments provided by the user
    void initialiase(int argc, char* argv[]);

    // Retrieve the name of the library to evaluate
    const std::string& get_library_name() const { return m_library_name; }

    // Path to the graphlog with the updates to perform (aging2 experiment)
    const std::string& get_update_log() const { return m_update_log; }

    // Generate an instance of the graph library to evaluate
    std::unique_ptr<library::Interface> generate_graph_library();

    // Whether the graph is directed or undirected
    bool is_graph_directed() const { return m_graph_directed; }

    // Whether to validate the execution results of the Graphalytics algorithms
    bool validate_output() const { return m_validate_output; }

    // Coefficient for the surplus of updates to perform (noise) w.r.t. the final graph  to load
    double coefficient_aging() const{ return m_coeff_aging; }

    // Number of repetitions of the same experiment (when applicable)
    uint64_t num_repetitions() const { return m_num_repetitions; }

    // Get the number of threads to use
    int num_threads(ThreadsType type) const;

    // The path for the graph to load
    const std::string& get_path_graph() const { return m_path_graph_to_load; }

    // The budget to complete a Graphalytics algorithm, in seconds (e.g. LCC should terminate by get_timeout_per_operation() seconds)
    uint64_t get_timeout_per_operation() const { return m_timeout_seconds; }

    // Get the expansion factor in the aging experiment for the edges in the graph
    double get_ef_edges() const { return m_ef_edges; }

    // Get the expansion factor in the aging experiment for the vertices in the graph
    double get_ef_vertices() const { return m_ef_vertices; }

    // Get the frequency to build a new snapshot, in milliseconds
    uint64_t get_build_frequency() const{ return m_build_frequency; }

    // Check whether the configuration/results need to be stored into a database
    bool has_database() const;

    // Retrieve the handle to the database connection, where the final results of the experiments are stored
    common::Database* db();

    // Save the configuration properties into the database
    void save_parameters();

    // Random seed, used in various places in the experiments
    uint64_t seed() const { return m_seed; };

    // Get the max weight that can be assigned by the reader to
    double max_weight() const { return m_max_weight; }

    // Retrieve the path to the database
    const std::string& get_database_path() const { return m_database_path; }
};

