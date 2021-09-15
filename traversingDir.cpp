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
    
    using FILE_CONFIG_TYPE = std::list<std::tuple<std::string, uint16_t, std::string, uint32_t>>; // path, depthMax, ecuid, type

    std::list<updLogFileInfo>
    LogUploadStateUploading::collectFilesUnderDir(const FILE_CONFIG_TYPE& configs)
    {
        std::list<updLogFileInfo> list;
        for (auto config : configs) {
            log::idc::traversingDir(std::get<0>(config).c_str(), std::get<1>(config), [&] (const FTSENT* fts) {
                if (fts->fts_info == FTS_F) {
                    updLogFileInfo info;
                    info.path            = android::base::StringPrintf("%s/%s", fts->fts_path, fts->fts_name);
                    info.originalName    = std::string(fts->fts_name);
                    info.ecu             = std::get<2>(config);
                    info.type            = (LOG_UPD_IDCM_DATA_TYPE)std::get<3>(config);
                    info.timestamp_ms    = (fts->fts_statp->st_atim.tv_sec * 1000) + ns2ms(fts->fts_statp->st_atim.tv_nsec);
                    info.size            = fts->fts_statp->st_size;

                    list.push_back(info);
                    LOGSERI(TAG, "collect %s", info.path.c_str());
                }
            });
        }
        list.sort();
        return list;
    }
