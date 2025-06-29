#pragma once

#include <chrono>
#include <fstream>
#include <functional>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>

namespace TrainTracks {
    template <typename T> constexpr int sgn(T val) {
        return (T(0) < val) - (val < T(0));
    }    

    inline std::string_view trim(const std::string_view v) {
        std::string_view s = v;
        while (!s.empty() && s.front() == ' ') {
            s.remove_prefix(1);
        }

        while (!s.empty() && s.back() == ' ') {
            s.remove_suffix(1);
        }
        return s;
    }

    inline bool startsWith(const std::string_view s, const std::string_view p) {
        if  (s.size() >= p.size()) {
            const auto ss = s.substr(0, p.size());
            return ss.compare(p) == 0;
        }
        return false;
    }

    inline bool startsWith(const std::string& s, const std::string_view p) {
        return startsWith(std::string_view{s.data(), s.size()}, p);
    }
    inline bool getline(std::istream& s, std::string& out, const std::string_view delims) {
        char c;
        out.resize(0);
        while (s.good() && (c = s.get())) {
            if (delims.find(c) != std::string_view::npos) {
                if (out.empty()) {
                    continue;
                }
                return true;
            }
            if (c > 0) {
                out.append(&c, 1);
            }
        }
        return !out.empty() || s.good();
    }
    inline bool getline(std::istream& s, std::string& out, const char delim) {
        return getline(s, out, std::string_view(&delim, 1));
    }
    inline bool getline(std::istream& s, std::string& out) {
        char c;
        out.resize(0);
        while (s.good() && (c = s.get())) {
            switch (c) {
                case '\r':
                case '\n':
                    if (out.empty()) { continue; }
                    return true;
                default:
                    if (c > 0) {
                        out.append(&c, 1);
                    }
                    break;
            }
        }
        return !out.empty() || s.good();
    }

    using UnaryIntFunction = std::function<void(int)>;
    inline void parse_as_integers(std::istream& s, const char delim, UnaryIntFunction op) {
        std::string l;
        while (getline(s, l, delim)) {
            try {
                int n = std::stoi(l);
                op(n);
            } catch (...) { }
        }
    }
    inline void parse_as_integers(std::istream& s, const std::string_view delims, UnaryIntFunction op) {
        std::string l;
        while (getline(s, l, delims)) {
            try {
                int n = std::stoi(l);
                op(n);
            } catch (...) { }
        }
    }
    inline void parse_as_integers(std::istream& s, UnaryIntFunction op) {
        std::string l;
        while (getline(s, l)) {
            try {
                int n = std::stoi(l);
                op(n);
            } catch (...) { }
        }
    }
    inline void parse_as_integers(const std::string& s, const char delim, UnaryIntFunction op) {
        std::stringstream ss(s);
        std::string l;
        while (getline(ss, l, delim)) {
            try {
                int n = std::stoi(l);
                op(n);
            } catch (...) { }
        }
    }
    inline void parse_as_integers(const std::string& s, const std::string_view delims, UnaryIntFunction op) {
        std::stringstream ss(s);
        std::string l;
        while (getline(ss, l, delims)) {
            try {
                int n = std::stoi(l);
                op(n);
            } catch (...) { }
        }
    }

    inline std::ostream& bold_on(std::ostream& os) {
        return os << "\e[1m";
    }

    inline std::ostream& bold_off(std::ostream& os) {
        return os << "\e[0m";
    }

    inline std::ostream& cls(std::ostream& os) {
        return os << "\033[2J\033[1;1H";
    }

    inline std::ostream& reset(std::ostream& os) {
        return os << "\033[1;1H";
    }

    class AutoTimer {
    private:
        const bool display_;
        std::chrono::time_point<std::chrono::high_resolution_clock> start_;
        const std::string name_;
    public:
        AutoTimer(const char *name, bool display)
            : start_(std::chrono::high_resolution_clock::now())
            , name_(name)
            , display_(display)
        { }

        AutoTimer(const char *name)
            : AutoTimer(name, true)
        { }

        AutoTimer()
            :  AutoTimer("")
        { }

        ~AutoTimer() {
            calculate_time(true);
        }

        double elapsed() const {
            return calculate_time(display_);
        }

        void reset() {
            start_ = std::chrono::high_resolution_clock::now();
        }

    private:
        // Needs to be a lambda due to use of auto
        double calculate_time (bool display) const {
            const auto end = std::chrono::high_resolution_clock::now();

            // Calculating total time taken by the program.
            double time_taken = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start_).count();
            time_taken *= 1e-9;

            if (display) {
                std::cout << "Elapsed" << (name_.empty() ? "" : " " + name_) << ": " << std::fixed << time_taken << std::setprecision(9) << " sec" << std::endl;
            }
        
            return time_taken;
        }

    };
            
}