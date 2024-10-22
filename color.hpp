class dye {
public:
    // Color escape codes
    static std::string reset() {
        return "\033[0m";
    }

    static std::string grey(const std::string& text) {
        return "\033[90m" + text + reset();
    }

    static std::string red(const std::string& text) {
        return "\033[91m" + text + reset();
    }

    static std::string green(const std::string& text) {
        return "\033[92m" + text + reset();
    }

    static std::string yellow(const std::string& text) {
        return "\033[93m" + text + reset();
    }

    static std::string blue(const std::string& text) {
        return "\033[94m" + text + reset();
    }

    static std::string magenta(const std::string& text) {
        return "\033[95m" + text + reset();
    }

    static std::string cyan(const std::string& text) {
        return "\033[96m" + text + reset();
    }

    static std::string white(const std::string& text) {
        return "\033[97m" + text + reset();
    }
};