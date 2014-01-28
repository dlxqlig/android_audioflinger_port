/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __CUTILS_SCHED_POLICY_H
#define __CUTILS_SCHED_POLICY_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SP_DEFAULT    = -1,
    SP_BACKGROUND = 0,
    SP_FOREGROUND = 1,
    SP_SYSTEM     = 2,  // can't be used with set_sched_policy()
    SP_AUDIO_APP  = 3,
    SP_AUDIO_SYS  = 4,
    SP_CNT,
    SP_MAX        = SP_CNT - 1,
    SP_SYSTEM_DEFAULT = SP_FOREGROUND,
} SchedPolicy;

int set_sched_policy(int tid, SchedPolicy policy);
int get_sched_policy(int tid, SchedPolicy *policy);
const char *get_sched_policy_name(SchedPolicy policy);

#ifdef __cplusplus
}
#endif

#endif /* __CUTILS_SCHED_POLICY_H */ 
