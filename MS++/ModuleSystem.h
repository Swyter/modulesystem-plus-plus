#pragma once

#include "CPyObject.h"
#include <algorithm>
#include <ostream>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

struct ScriptErrorContext
{
	int line;
	int statement;
	std::string context;
	std::string error;
};

struct Variable
{
	int index;
	int usages;
	int assignments;
	bool compat;
};

struct QuickString
{
	int index;
	std::string value;
};

class CompileException
{
public:
	CompileException(const std::string &error) : m_error(error)
	{
	};

	const std::string &GetText()
	{
		return m_error;
	}

private:
	std::string m_error;
};

#define OPCODE(obj) ((unsigned long long)obj & opcode_mask)
#define opcode_mask 0xFFFFFFF
#define lhs 0x1
#define ghs 0x2
#define cf  0x4
#define opmask_register        (1ULL  << 56)
#define opmask_global_variable (2ULL  << 56)
#define opmask_local_variable  (17ULL << 56)
#define opmask_quick_string    (22ULL << 56)

#define msf_strict                  0x1
#define msf_obfuscate_global_vars   0x2
#define msf_obfuscate_dialog_states 0x4
#define msf_obfuscate_scripts       0x8
#define msf_skip_id_files           0x10 // TODO

class ModuleSystem
{
public:
	void Compile(unsigned long long flags = 0);

private:
	void DoCompile();
	CPyList AddModule(const std::string &module_name, const std::string &list_name, const std::string &prefix);
	CPyList AddModule(const std::string &module_name, const std::string &prefix);
	int GetId(const CPyObject &obj);
	long long ParseOperand(const CPyObject &statement, int pos);
	void Error(const std::string &text);
	void Warning(const std::string &text);
	void WriteAnimations();
	void WriteDialogs();
	void WriteFactions();
	void WriteGlobalVars();
	void WriteInfoPages();
	void WriteItems();
	void WriteMapIcons();
	void WriteMenus();
	void WriteMeshes();
	void WriteMissionTemplates();
	void WriteMusic();
	void WritePresentations();
	void WriteQuests();
	void WriteQuickStrings();
	void WriteParticleSystems();
	void WriteParties();
	void WritePartyTemplates();
	void WritePostEffects();
	void WriteSceneProps();
	void WriteScenes();
	void WriteScripts();
	void WriteSimpleTriggers();
	void WriteSkills();
	void WriteSkins();
	void WriteSounds();
	void WriteStrings();
	void WriteTableaus();
	void WriteTriggers();
	void WriteTroops();
	void WriteSimpleTriggerBlock(const CPyObject &simple_trigger_block, std::ostream &stream);
	void WriteSimpleTrigger(const CPyObject &simple_trigger, std::ostream &stream);
	void WriteTriggerBlock(const CPyObject &trigger_block, std::ostream &stream);
	void WriteTrigger(const CPyObject &trigger, std::ostream &stream);
	void WriteStatementBlock(const CPyObject &statement_block, std::ostream &stream);
	void WriteStatement(const CPyObject &statement, std::ostream &stream);

private:
	std::string m_path;
	unsigned long long m_flags;
	std::map<std::string, std::map<std::string, int> > m_ids;
	CPyList m_animations;
	CPyList m_dialogs;
	CPyList m_factions;
	CPyList m_game_menus;
	CPyList m_info_pages;
	CPyList m_items;
	CPyList m_map_icons;
	CPyList m_meshes;
	CPyList m_music;
	CPyList m_mission_templates;
	CPyList m_particle_systems;
	CPyList m_parties;
	CPyList m_party_templates;
	CPyList m_postfx;
	CPyList m_presentations;
	CPyList m_quests;
	CPyList m_scene_props;
	CPyList m_scenes;
	CPyList m_scripts;
	CPyList m_simple_triggers;
	CPyList m_skills;
	CPyList m_skins;
	CPyList m_sounds;
	CPyList m_strings;
	CPyList m_tableau_materials;
	CPyList m_triggers;
	CPyList m_troops;
	std::map<int, unsigned int> m_operations;
	std::map<std::string, Variable> m_global_vars;
	std::map<std::string, Variable> m_local_vars;
	std::map<std::string, QuickString> m_quick_strings;
	std::vector<std::string> m_warnings;
};
