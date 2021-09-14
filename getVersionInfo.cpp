    static inline std::string version() {
        static auto pack = [] (const std::string& prop) {
            auto value = android::base::GetProperty(prop, "unknow");
            return android::base::StringPrintf("[%s]: [%s]\n", prop.c_str(), value.c_str());
        };

        std::string r;
        r += pack("ro.build.date");
        r += pack("ro.build.display.id");

        r += "--------\n";

        return r;
    }
    
