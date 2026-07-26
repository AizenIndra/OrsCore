/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

void AddSC_MakGoraCore();
void AddSC_MakGoraCommands();
void AddSC_MakGoraChampion();

// Module folder: mod-makgora → Addmod_makgoraScripts()
void Addmod_makgoraScripts()
{
    AddSC_MakGoraCore();
    AddSC_MakGoraCommands();
    AddSC_MakGoraChampion();
}
