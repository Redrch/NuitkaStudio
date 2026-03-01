/*************************************************************************
 *
 * Created by redrch on 2026/2/13.
 *
 * Please do not arbitrarily modify the content of this file,
 * as a large number of abbreviations can make the code difficult to read.
 * If you want to make changes to this file,
 * please explain the changes and the reasons for them to me via email.
 * Mandatory conditions for allowing modifications
 * (meeting all mandatory conditions do not guarantee that the modification will be approved):
 * 1. This data type is used more than 100 times throughout the project.
 * 2. The modification has been agreed upon by the author of this data type.
 * 3. Ideally, the alias you want to give this data type should be its abbreviation
 * (e.g., ProjectConfigManager -> PCM), not meaningless characters.

 * Email: redrch327@gmail.com
 * (Cannot guarantee a timely response)
 *
 *                                                           Redrch
 *                                                           2026/2/17
 *
*************************************************************************/

#ifndef PCM_H
#define PCM_H

#include "global_data.h"
#include "../utils/config.h"
#include "project_config_manager.h"

inline ProjectConfigManager &PCM = ProjectConfigManager::instance();
inline GlobalData &GDM = GlobalData::instance();
inline Config &config = Config::instance();
namespace GDIN = GlobalDataItemName;

#endif //PCM_H
