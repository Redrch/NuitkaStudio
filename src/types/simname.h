//
// Created by redrch on 2026/2/13.
//

#ifndef PCM_H
#define PCM_H

#include "project_config_manager.h"
#include "global_data.h"

inline ProjectConfigManager &PCM = ProjectConfigManager::instance();
inline GlobalData &GDM = GlobalData::instance();

inline const QString& npf_file_path = "npf_file_path";

#endif //PCM_H
