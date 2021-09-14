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
    
    // traversing file hierarchies
    static inline void traversingDir(const char* dir, uint16_t depthMax, const std::function<void(const FTSENT*)>& callback) {
        FTS* fhandle = NULL;
        FTSENT* child = NULL;
        FTSENT* parent = NULL;

        const char* path[] = { dir, NULL };
        fhandle = fts_open((char* const*)path, FTS_COMFOLLOW, [] (const FTSENT ** first, const FTSENT ** second) {
            return strcmp((*first)->fts_name, (*second)->fts_name);
        });
        if (fhandle != NULL) {
            while ((parent = fts_read(fhandle)) != NULL) {
                if (parent->fts_level > depthMax) {
                    break;
                }
                child = fts_children(fhandle, 0);
                while ((child != NULL)) {
                    callback(child);
                    child = child->fts_link;
                }
            }
            fts_close(fhandle);
        }
    }
    
    // 回调函数处理
    traversingDir(std::get<0>(config).c_str(), std::get<1>(config), [&] (const FTSENT* fts) {
            if (fts->fts_info == FTS_F) {
                updLogFileInfo info;
                info.path            = android::base::StringPrintf("%s/%s", fts->fts_path, fts->fts_name);
                info.originalName    = std::string(fts->fts_name);
                info.ecu             = std::get<2>(config);
                info.type            = (LOG_UPD_IDCM_DATA_TYPE)std::get<3>(config);
                info.timestamp_ms    = (fts->fts_statp->st_atim.tv_sec * 1000) + ns2ms(fts->fts_statp->st_atim.tv_nsec);
                info.size            = fts->fts_statp->st_size;
                map.insert(LogsBrowserMachine::FILESLISTPAIR(index++, info));
            }
        }
