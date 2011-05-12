#include "ModuleSystem.h"

std::string encode_str(const std::string &str)
{
	std::string text = str;

	std::replace(text.begin(), text.end(), ' ', '_');
	std::replace(text.begin(), text.end(), '\t', '_');
	return text;
}

std::string encode_full(const std::string &str)
{
	std::string text = encode_str(str);

	std::replace(text.begin(), text.end(), '\'', '_');
	std::replace(text.begin(), text.end(), '`', '_');
	std::replace(text.begin(), text.end(), '(', '_');
	std::replace(text.begin(), text.end(), ')', '_');
	std::replace(text.begin(), text.end(), '-', '_');
	std::replace(text.begin(), text.end(), ',', '_');
	std::replace(text.begin(), text.end(), '|', '_');
	return text;
}

std::string encode_id(const std::string &str)
{
	std::string text = encode_full(str);

	std::transform(text.begin(), text.end(), text.begin(), ::tolower);
	return text;
}

void ModuleSystem::Compile(unsigned long long flags)
{
	m_flags = flags;

	try
	{
		DoCompile();
	}
	catch (CPyException e)
	{
		throw CompileException(e.GetText());
	}
}

void ModuleSystem::DoCompile()
{
	std::cout << "Initializing compiler..." << std::endl;

	CPyModule header_operations("header_operations");
	CPyIter lhs_operations_iter = header_operations.GetAttr("lhs_operations").GetIter();
	CPyIter ghs_operations_iter = header_operations.GetAttr("global_lhs_operations").GetIter();
	CPyIter cf_operations_iter = header_operations.GetAttr("can_fail_operations").GetIter();

	while (lhs_operations_iter.HasNext())
	{
		m_operations[OPCODE(lhs_operations_iter.Next().AsLong())] |= lhs;
	}

	while (ghs_operations_iter.HasNext())
	{
		m_operations[OPCODE(ghs_operations_iter.Next().AsLong())] |= ghs;
	}

	while (cf_operations_iter.HasNext())
	{
		m_operations[OPCODE(cf_operations_iter.Next().AsLong())] |= cf;
	}

	CPyModule module_info("module_info");

	m_path = module_info.GetAttr("export_dir").Str();
	m_path = "C:\\Users\\FX.Net\\Desktop\\"; // TODO: remove

	std::ifstream global_var_stream("variables.txt");

	if (!global_var_stream.is_open())
		global_var_stream.open(m_path + "variables.txt");

	if (global_var_stream.is_open())
	{
		int i = 0;

		while (global_var_stream)
		{
			std::string global_var;

			global_var_stream >> global_var;

			if (!global_var.empty() && m_global_vars.find(global_var) == m_global_vars.end())
			{
				m_global_vars[global_var].index = i++;
				m_global_vars[global_var].compat = true;
			}
		}
	}

	std::cout << "Loading modules..." << std::endl;

	m_animations = AddModule("animations", "anim");
	m_dialogs = AddModule("dialogs", "");
	m_factions = AddModule("factions", "fac");
	m_game_menus = AddModule("game_menus", "mnu");
	m_info_pages = AddModule("info_pages", "ip");
	m_items = AddModule("items", "itm");
	m_map_icons = AddModule("map_icons", "icon");
	m_meshes = AddModule("meshes", "mesh");
	m_music = AddModule("music", "tracks", "track");
	m_mission_templates = AddModule("mission_templates", "mt");
	m_particle_systems = AddModule("particle_systems", "psys");
	m_parties = AddModule("parties", "p");
	m_party_templates = AddModule("party_templates", "pt");
	m_postfx = AddModule("postfx", "postfx_params", "pfx");
	m_presentations = AddModule("presentations", "prsnt");
	m_quests = AddModule("quests", "qst");
	m_scene_props = AddModule("scene_props", "spr");
	m_scenes = AddModule("scenes", "scn");
	m_scripts = AddModule("scripts", "script");
	m_simple_triggers = AddModule("simple_triggers", "");
	m_skills = AddModule("skills", "skl");
	m_skins = AddModule("skins", "skn");
	m_sounds = AddModule("sounds", "snd");
	m_strings = AddModule("strings", "str");
	m_tableau_materials = AddModule("tableau_materials", "tableaus", "tableau");
	m_triggers = AddModule("triggers", "");
	m_troops = AddModule("troops", "trp");

	std::cout << "Compiling..." << std::endl;
	
	WriteStrings();
	WriteSkills();
	WriteMusic();
	WriteAnimations();
	WriteMeshes();
	WriteSounds();
	WriteSkins();
	WriteMapIcons();
	WriteFactions();
	WriteItems();
	WriteScenes();
	WriteTroops();
	WriteParticleSystems();
	WriteSceneProps();
	WriteTableaus();
	WritePresentations();
	WritePartyTemplates();
	WriteParties();
	WriteQuests();
	WriteInfoPages();
	WriteScripts();
	WriteMissionTemplates();
	WriteMenus();
	WriteSimpleTriggers();
	WriteTriggers();
	WriteDialogs();
	WritePostEffects();
	
	WriteQuickStrings();
	WriteGlobalVars();

	std::map<std::string, Variable>::const_iterator it;

	for (it = m_global_vars.begin(); it != m_global_vars.end(); ++it)
	{
		if (!it->second.compat && it->second.assignments == 0)
			Warning("usage of unassigned global variable $" + it->first);

		if (!it->second.compat && it->second.usages == 0)
			Warning("unused global variable $" + it->first);
	}

	for (size_t i = 0; i < m_warnings.size(); ++i)
	{
		std::cout << m_warnings[i] << std::endl;
	}
};

CPyList ModuleSystem::AddModule(const std::string &module_name, const std::string &list_name, const std::string &prefix)
{
	CPyModule module("module_" + module_name);
	CPyList list = module.GetAttr(list_name);

	if (!prefix.empty())
	{
		int num_entries = list.Size();

		for (int i = 0; i < num_entries; ++i)
		{
			CPyObject item = list.GetItem(i);
			std::string name;
			
			if (item.IsTuple())
				name = item.AsTuple().GetItem(0).AsString();
			else if (item.IsList())
				name = item.AsList().GetItem(0).AsString();
			
			std::string prefix_lower = prefix;
			
			std::transform(prefix_lower.begin(), prefix_lower.end(), prefix_lower.begin(), ::tolower);
			std::transform(name.begin(), name.end(), name.begin(), ::tolower);
			m_ids[prefix_lower][name] = i;
		}
	}

	return list;
};

CPyList ModuleSystem::AddModule(const std::string &module_name, const std::string &prefix)
{
	return AddModule(module_name, module_name, prefix);
};

int ModuleSystem::GetId(const CPyObject &obj) // TODO: use
{
	if (obj.IsString())
	{
		std::string str = obj.AsString();
		int underscore_pos = str.find('_');

		if (underscore_pos < 0)
			Error("invalid identifier " + str);
			
		std::string prefix = str.substr(0, underscore_pos);
		std::string value = str.substr(underscore_pos + 1);
			
		std::transform(prefix.begin(), prefix.end(), prefix.begin(), ::tolower);
		std::transform(value.begin(), value.end(), value.begin(), ::tolower);

		if (m_ids.find(prefix) == m_ids.end())
			Error("unrecognized identifier type " + prefix);

		if (m_ids[prefix].find(value) == m_ids[prefix].end())
			Error("unrecognized identifier " + str);

		return m_ids[prefix][value];
	}
	else if (obj.IsLong())
	{
		return obj.AsLong();
	}
	
	Error("unrecognized identifier type " + (std::string)obj.Type().Str() + " for " + (std::string)obj.Str());
	return -1;
}

long long ModuleSystem::ParseOperand(const CPyObject &statement, int pos)
{
	CPyObject operand = statement.GetItem(pos);

	if (operand.IsTuple() && operand.Len() == 1)
		operand = operand.AsTuple().GetItem(0);
	else if (operand.IsList() && operand.Len() == 1)
		operand = operand.AsList().GetItem(0);

	if (operand.IsString())
	{
		std::string str = operand.AsString();
		
		if (str[0] == ':')
		{
			std::string value = str.substr(1);
			int index;

			if (m_local_vars.find(value) == m_local_vars.end())
			{
				int index = m_local_vars.size();

				if (pos != 1 || !(m_operations[OPCODE(statement.GetItem(0).AsLong())] & lhs))
					Warning("usage of unassigned local variable :" + value);
				
				m_local_vars[value].index = index;
				m_local_vars[value].assignments = 1;
				m_local_vars[value].usages = 0;
			}
			else
			{
				if (pos == 1 && m_operations[OPCODE(statement.GetItem(0).AsLong())] & lhs)
					m_local_vars[value].assignments++;
				else
					m_local_vars[value].usages++;

				index = m_local_vars[value].index;
			}

			return index | opmask_local_variable;
		}
		else if (str[0] == '$')
		{
			std::string value = str.substr(1);
			int index;

			if (value == "server_mission_timer_while_player_joined")
			{
				int a = 2;
				int b = std::min<int>(1,2);

				std::string lol;

				lol = lol.substr(0,1);
			}

			if (m_global_vars.find(value) == m_global_vars.end())
			{
				int index = m_global_vars.size();

				m_global_vars[value].index = index;

				if (pos == 1 && m_operations[OPCODE(statement.GetItem(0).AsLong())] & (lhs | ghs))
					m_global_vars[value].assignments = 1;
				else
					m_global_vars[value].usages = 1;
			}
			else
			{
				if (pos == 1 && m_operations[OPCODE(statement.GetItem(0).AsLong())] & (lhs | ghs))
					m_global_vars[value].assignments++;
				else
					m_global_vars[value].usages++;
				
				m_global_vars[value].compat = false;
				index = m_global_vars[value].index;
			}

			return index | opmask_global_variable;
		}
		else if (str[0] == '@')
		{
			std::string id = encode_full(str.substr(1));
			std::string text = encode_str(str.substr(1));
			size_t auto_id_len = std::min<int>(20, text.length());
			std::string auto_id;

			do
			{
				auto_id = "qstr_" + id.substr(0, auto_id_len++);
			}
			while (auto_id_len <= id.length() && (m_quick_strings.find(auto_id) != m_quick_strings.end() && m_quick_strings[auto_id].value != text));

			if (auto_id_len > id.length())
			{
				std::string new_auto_id;
				int i = 1;

				do
				{
					std::ostringstream oss;

					oss << auto_id << i++;
					new_auto_id = oss.str();
				}
				while (m_quick_strings.find(new_auto_id) != m_quick_strings.end() && m_quick_strings[new_auto_id].value != text);

				auto_id = new_auto_id;
			}

			if (m_quick_strings.find(auto_id) == m_quick_strings.end())
			{
				m_quick_strings[auto_id].index = m_quick_strings.size();
				m_quick_strings[auto_id].value = text;
			}

			return m_quick_strings[auto_id].index | opmask_quick_string;
		}
		else
		{
			return GetId(operand);
		}
	}
	else if (operand.IsLong())
	{
		// TODO: if obfuscating add random/fixed opmask. Not for registers.
		return operand.AsLong();
	}
	
	Error("unrecognized operand type " + (std::string)operand.Type().Str() + " for " + (std::string)operand.Str());
	return -1;
}

void ModuleSystem::Error(const std::string &text)
{
	throw CompileException(text);
}

void ModuleSystem::Warning(const std::string &text)
{
	if (m_flags & msf_strict)
		Error(text);

	m_warnings.push_back(text);
}

void ModuleSystem::WriteAnimations()
{
	std::ofstream stream(m_path + "actions.txt");
	CPyIter iter = m_animations.GetIter();

	while (iter.HasNext())
	{
		CPyObject animation = iter.Next();
		
		stream << encode_str(animation.GetItem(0).AsString()) << " ";
		stream << animation.GetItem(1) << " ";
		stream << animation.GetItem(2) << " ";

		int num_sequences = animation.Len() - 3;

		stream << num_sequences << " ";

		for (int i = 0; i < num_sequences; ++i)
		{
			CPyObject sequence = animation.GetItem(i + 3);
			
			stream << std::endl;
			stream << sequence.GetItem(0) << " ";
			stream << encode_str(sequence.GetItem(1).AsString()) << " ";
			stream << sequence.GetItem(2) << " ";
			stream << sequence.GetItem(3) << " ";
			stream << sequence.GetItem(4) << " ";

			if (sequence.Len() > 5)
				stream << sequence.GetItem(5) << " ";
			else
				stream << "0 ";

			if (sequence.Len() > 6)
			{
				stream << sequence.GetItem(6).GetItem(0) << " ";
				stream << sequence.GetItem(6).GetItem(1) << " ";
				stream << sequence.GetItem(6).GetItem(2) << " ";
			}
			else
			{
				stream << "0.0 ";
				stream << "0.0 ";
				stream << "0.0 ";
			}

			if (sequence.Len() > 7)
				stream << sequence.GetItem(7) << " ";
			else
				stream << "0.0 ";
		}

		stream << std::endl;
	}
}

void ModuleSystem::WriteDialogs()
{
	std::ofstream states_stream(m_path + "dialog_states.txt");
	CPyIter states_iter = m_dialogs.GetIter();
	std::map<std::string, int> states;
	int num_states = 15;
	std::string default_states[] = {
		"start",
		"party_encounter",
		"prisoner_liberated",
		"enemy_defeated",
		"party_relieved",
		"event_triggered",
		"close_window",
		"trade",
		"exchange_members",
		"trade_prisoners",
		"buy_mercenaries",
		"view_char",
		"training",
		"member_chat",
		"prisoner_chat",
	};

	for (int i = 0; i < num_states; ++i)
	{
		states[default_states[i]] = i;
		states_stream << default_states[i] << std::endl;
	}

	while (states_iter.HasNext())
	{
		CPyObject sentence = states_iter.Next();
		std::string output_token = sentence.GetItem(4).AsString();

		if (states.find(output_token) == states.end())
		{
			states[output_token] = num_states++;

			if (m_flags & msf_obfuscate_dialog_states)
				states_stream << "state_" << states[output_token] << std::endl;
			else
				states_stream << output_token << std::endl;
		}
	}

	std::ofstream stream(m_path + "conversation.txt");
	CPyIter	iter = m_dialogs.GetIter();
	std::map<std::string, int> dialog_ids;

	stream << "dialogsfile version 2" << std::endl;
	stream << m_dialogs.Size() << std::endl;

	while (iter.HasNext())
	{
		CPyObject sentence = iter.Next();
		std::string input_token = sentence.GetItem(1).AsString();
		std::string output_token = sentence.GetItem(4).AsString();

		if (states.find(input_token) == states.end())
			Error("input token not found: " + input_token);

		std::string auto_id = "dlga_" + encode_id(input_token) + ":" + encode_id(output_token);
		std::string new_auto_id = auto_id;
		int i = 1;
		
		while (dialog_ids.find(new_auto_id) != dialog_ids.end())
		{
			std::ostringstream oss;
			
			oss << auto_id << "." << i++;
			new_auto_id = oss.str();
		}
		
		auto_id = new_auto_id;
		dialog_ids[auto_id] = 0;

		stream << auto_id << " ";
		stream << sentence.GetItem(0) << " ";
		stream << states[input_token] << " ";
		WriteStatementBlock(sentence.GetItem(2), stream);

		std::string text = encode_str(sentence.GetItem(3).AsString());

		if (text.empty())
			text = "NO_TEXT";

		stream << text << " ";
		stream << states[output_token] << " ";
		WriteStatementBlock(sentence.GetItem(5), stream);

		if (sentence.Len() > 6)
			stream << encode_str(sentence.GetItem(6).AsString()) << " ";
		else
			stream << "NO_VOICEOVER ";

		stream << std::endl;
	}
}

void ModuleSystem::WriteFactions()
{
	std::ofstream stream(m_path + "factions.txt");
	CPyIter iter = m_factions.GetIter();
	
	std::map<std::string, int> faction_ids;
	std::map<int, std::map<int, double> > relations;

	for (int i = 0; i < m_factions.Len(); ++i)
	{
		faction_ids[m_factions.GetItem(i).GetItem(0).AsString()] = i;
	}

	while (iter.HasNext())
	{
		CPyObject faction = iter.Next();
		int faction_id = faction_ids[faction.GetItem(0).AsString()];
		CPyIter relation_iter = faction.GetItem(4).GetIter();

		relations[faction_id][faction_id] = faction.GetItem(3).AsFloat();

		while (relation_iter.HasNext())
		{
			CPyObject relation = relation_iter.Next();
			int other_id;

			if (relation.GetItem(0).IsString())
				other_id = faction_ids[relation.GetItem(0).AsString()];
			else
				other_id = relation.GetItem(0).AsLong();

			double value = relation.GetItem(1).AsFloat();

			relations[faction_id][other_id] = value;

			if (relations[other_id][faction_id] == 0.0)
				relations[other_id][faction_id] = value;
		}
	}

	iter = m_factions.GetIter();
	stream << "factionsfile version 1" << std::endl;
	stream << m_factions.Size() << std::endl;

	while (iter.HasNext())
	{
		CPyObject faction = iter.Next();
		int faction_id = faction_ids[faction.GetItem(0).AsString()];
		
		stream << "fac_" << encode_id(faction.GetItem(0).AsString()) << " ";
		stream << encode_str(faction.GetItem(1).AsString()) << " ";
		stream << faction.GetItem(2) << " ";

		if (faction.Len() > 6)
			stream << faction.GetItem(6) << " ";
		else
			stream << 0xAAAAAA << " ";

		for (int i = 0; i < m_factions.Len(); ++i)
		{
			stream << relations[faction_id][i] << " ";
		}

		if (faction.Len() > 5)
		{
			CPyObject ranks = faction.GetItem(5);
			CPyIter rank_iter = ranks.GetIter();

			stream << ranks.Len() << " ";

			while (rank_iter.HasNext())
			{
				stream << encode_str(rank_iter.Next().AsString()) << " ";
			}
		}

		stream << std::endl;
	}
}

void ModuleSystem::WriteGlobalVars()
{
	std::ofstream stream(m_path + "variables.txt");
	std::vector<std::string> global_vars(m_global_vars.size());
	std::map<std::string, Variable>::const_iterator it;

	for (it = m_global_vars.begin(); it != m_global_vars.end(); ++it)
	{
		global_vars[it->second.index] = it->first;
	}

	for (size_t i = 0; i < global_vars.size(); ++i)
	{
		if (m_flags & msf_obfuscate_global_vars)
			stream << "global_var_" << i << std::endl;
		else
			stream << global_vars[i] << std::endl;
	}
}

void ModuleSystem::WriteInfoPages()
{
	std::ofstream stream(m_path + "info_pages.txt");
	CPyIter iter = m_info_pages.GetIter();

	stream << "infopagesfile version 1" << std::endl;
	stream << m_info_pages.Size() << std::endl;

	while (iter.HasNext())
	{
		CPyObject info_page = iter.Next();
		
		stream << "ip_" << encode_id(info_page.GetItem(0).AsString()) << " ";
		stream << encode_str(info_page.GetItem(1).AsString()) << " ";
		stream << encode_str(info_page.GetItem(2).AsString()) << " ";
		stream << std::endl;
	}
}

void ModuleSystem::WriteItems()
{
	std::ofstream stream(m_path + "item_kinds_1.txt");
	CPyIter iter = m_items.GetIter();
	CPyModule header_items("header_items");
	CPyObject get_weight = header_items.GetAttr("get_weight");
	CPyObject get_abundance = header_items.GetAttr("get_abundance");
	CPyObject get_head_armor = header_items.GetAttr("get_head_armor");
	CPyObject get_body_armor = header_items.GetAttr("get_body_armor");
	CPyObject get_leg_armor = header_items.GetAttr("get_leg_armor");
	CPyObject get_difficulty = header_items.GetAttr("get_difficulty");
	CPyObject get_hit_points = header_items.GetAttr("get_hit_points");
	CPyObject get_speed_rating = header_items.GetAttr("get_speed_rating");
	CPyObject get_missile_speed = header_items.GetAttr("get_missile_speed");
	CPyObject get_weapon_length = header_items.GetAttr("get_weapon_length");
	CPyObject get_max_ammo = header_items.GetAttr("get_max_ammo");
	CPyObject get_thrust_damage = header_items.GetAttr("get_thrust_damage");
	CPyObject get_swing_damage = header_items.GetAttr("get_swing_damage");

	stream << "itemsfile version 3" << std::endl;
	stream << m_items.Size() << std::endl;

	while (iter.HasNext())
	{
		CPyObject item = iter.Next();
		
		stream << "itm_" << encode_id(item.GetItem(0).AsString()) << " ";
		stream << encode_str(item.GetItem(1).AsString()) << " ";
		stream << encode_str(item.GetItem(1).AsString()) << " ";

		CPyObject variations = item.GetItem(2);
		int num_variations = variations.Len();

		if (num_variations > 16)
		{
			Warning("item variation count exceeds 16");
			num_variations = 16;
		}

		stream << num_variations << " ";

		for (int i = 0; i < num_variations; ++i)
		{
			CPyObject variation = variations.GetItem(i);
			
			stream << encode_str(variation.GetItem(0).AsString()) << " ";
			stream << variation.GetItem(1) << " ";
		}

		stream << item.GetItem(3) << " ";
		stream << item.GetItem(4) << " ";
		stream << item.GetItem(5) << " ";
		stream << item.GetItem(7) << " ";

		CPyTuple args(1);

		args.SetItem(0, item.GetItem(6));
		stream << get_weight.Call(args) << " ";
		stream << get_abundance.Call(args) << " ";
		stream << get_head_armor.Call(args) << " ";
		stream << get_body_armor.Call(args) << " ";
		stream << get_leg_armor.Call(args) << " ";
		stream << get_difficulty.Call(args) << " ";
		stream << get_hit_points.Call(args) << " ";
		stream << get_speed_rating.Call(args) << " ";
		stream << get_missile_speed.Call(args) << " ";
		stream << get_weapon_length.Call(args) << " ";
		stream << get_max_ammo.Call(args) << " ";
		stream << get_thrust_damage.Call(args) << " ";
		stream << get_swing_damage.Call(args) << " ";

		if (item.Len() > 9)
		{
			CPyObject factions = item.GetItem(9);
			int num_factions = factions.Len();

			if (num_factions > 16)
			{
				Warning("item faction count exceeds 16");
				num_factions = 16;
			}

			stream << num_factions << " ";

			for (int i = 0; i < num_factions; ++i)
			{
				stream << factions.GetItem(i) << " ";
			}
		}
		else
		{
			stream << "0 ";
		}

		if (item.Len() > 8)
			WriteSimpleTriggerBlock(item.GetItem(8), stream);
		else
			stream << "0" << std::endl;
	}
}

void ModuleSystem::WriteMapIcons()
{
	std::ofstream stream(m_path + "map_icons.txt");
	CPyIter iter = m_map_icons.GetIter();

	stream << "map_icons_file version 1" << std::endl;
	stream << m_map_icons.Size() << std::endl;

	while (iter.HasNext())
	{
		CPyObject map_icon = iter.Next();
		
		stream << "prsnt_" << encode_id(map_icon.GetItem(0).AsString()) << " ";
		stream << map_icon.GetItem(1) << " ";
		stream << map_icon.GetItem(2) << " ";
		stream << map_icon.GetItem(3) << " ";
		stream << map_icon.GetItem(4) << " ";

		int trigger_pos;

		if (map_icon.Len() > 7)
		{
			stream << map_icon.GetItem(5) << " ";
			stream << map_icon.GetItem(6) << " ";
			stream << map_icon.GetItem(7) << " ";
			trigger_pos = 8;
		}
		else
		{
			stream << "0.0 ";
			stream << "0.0 ";
			stream << "0.0 ";
			trigger_pos = 5;
		}

		if (map_icon.Len() > trigger_pos)
			WriteSimpleTriggerBlock(map_icon.GetItem(trigger_pos), stream);

		stream << std::endl;
	}
}

void ModuleSystem::WriteMenus()
{
	std::ofstream stream(m_path + "menus.txt");
	CPyIter iter = m_game_menus.GetIter();

	stream << "menusfile version 1" << std::endl;
	stream << m_game_menus.Size() << std::endl;

	while (iter.HasNext())
	{
		CPyObject menu = iter.Next();

		stream << "menu_" << encode_id(menu.GetItem(0).AsString()) << " ";
		stream << menu.GetItem(1) << " ";
		stream << encode_str(menu.GetItem(2).AsString()) << " ";
		stream << encode_str(menu.GetItem(3).AsString()) << " ";
		WriteStatementBlock(menu.GetItem(4), stream);

		CPyObject items = menu.GetItem(5);
		CPyIter item_iter = items.GetIter();

		stream << items.Len() << " ";

		while (item_iter.HasNext())
		{
			CPyObject item = item_iter.Next();

			stream << std::endl;
			stream << encode_id(item.GetItem(0).AsString()) << " ";
			WriteStatementBlock(item.GetItem(1), stream);
			stream << encode_str(item.GetItem(2).AsString()) << " ";
			WriteStatementBlock(item.GetItem(3), stream);
			
			if (item.Len() > 4)
				stream << encode_str(item.GetItem(4).AsString()) << " ";
			else
				stream << ". ";
		}

		stream << std::endl;
	}
}

void ModuleSystem::WriteMeshes()
{
	std::ofstream stream(m_path + "meshes.txt");
	CPyIter iter = m_meshes.GetIter();

	stream << m_meshes.Size() << std::endl;

	while (iter.HasNext())
	{
		CPyObject mesh = iter.Next();
		
		stream << "mesh_" << encode_id(mesh.GetItem(0).AsString()) << " ";
		stream << mesh.GetItem(1) << " ";
		stream << encode_str(mesh.GetItem(2).AsString()) << " ";
		stream << mesh.GetItem(3) << " ";
		stream << mesh.GetItem(4) << " ";
		stream << mesh.GetItem(5) << " ";
		stream << mesh.GetItem(6) << " ";
		stream << mesh.GetItem(7) << " ";
		stream << mesh.GetItem(8) << " ";
		stream << mesh.GetItem(9) << " ";
		stream << mesh.GetItem(10) << " ";
		stream << mesh.GetItem(11) << " ";
		stream << std::endl;
	}
}

void ModuleSystem::WriteMissionTemplates()
{
	std::ofstream stream(m_path + "mission_templates.txt");
	CPyIter iter = m_mission_templates.GetIter();

	stream << "missionsfile version 1" << std::endl;
	stream << m_mission_templates.Size() << std::endl;

	while (iter.HasNext())
	{
		CPyObject mission_template = iter.Next();

		stream << "mst_" << encode_id(mission_template.GetItem(0).AsString()) << " ";
		stream << encode_id(mission_template.GetItem(0).AsString()) << " ";
		stream << mission_template.GetItem(1) << " ";
		stream << mission_template.GetItem(2) << " ";
		stream << encode_str(mission_template.GetItem(3).AsString()) << " ";

		CPyObject groups = mission_template.GetItem(4);
		CPyIter group_iter = groups.GetIter();

		stream << groups.Len() << " ";

		while (group_iter.HasNext())
		{
			CPyObject group = group_iter.Next();

			stream << std::endl;
			
			stream << group.GetItem(0) << " ";
			stream << group.GetItem(1) << " ";
			stream << group.GetItem(2) << " ";
			stream << group.GetItem(3) << " ";
			stream << group.GetItem(4) << " ";

			if (group.Len() > 5)
			{
				CPyObject overrides = group.GetItem(5);
				int num_overrides = overrides.Len();

				if (num_overrides > 8)
				{
					Warning("item override count exceeds 8");
					num_overrides = 8;
				}

				stream << num_overrides << " ";

				for (int i = 0; i < num_overrides; ++i)
				{
					stream << overrides.GetItem(i) << " ";
				}
			}
			else
			{
				stream << "0 ";
			}
		}
		
		stream << std::endl;
		WriteTriggerBlock(mission_template.GetItem(5), stream);
		stream << std::endl;
	}
}

void ModuleSystem::WriteMusic()
{
	std::ofstream stream(m_path + "music.txt");
	CPyIter iter = m_music.GetIter();

	stream << m_music.Size() << std::endl;

	while (iter.HasNext())
	{
		CPyObject track = iter.Next();
		unsigned long long flags = track.GetItem(2).AsLong();
		unsigned long long continue_flags = track.GetItem(3).AsLong();
		
		stream << encode_str(track.GetItem(1).AsString()) << " ";
		stream << flags << " ";
		stream << (flags | continue_flags) << " ";
		stream << std::endl;
	}
}

void ModuleSystem::WriteParticleSystems()
{
	std::ofstream stream(m_path + "particle_systems.txt");
	CPyIter iter = m_particle_systems.GetIter();

	stream << "particle_systemsfile version 1" << std::endl;
	stream << m_particle_systems.Size() << std::endl;

	while (iter.HasNext())
	{
		CPyObject particle_system = iter.Next();
		
		stream << "psys_" << encode_id(particle_system.GetItem(0).AsString()) << " ";
		stream << particle_system.GetItem(1) << " ";
		stream << encode_str(particle_system.GetItem(2).AsString()) << " ";
		stream << particle_system.GetItem(3) << " ";
		stream << particle_system.GetItem(4) << " ";
		stream << particle_system.GetItem(5) << " ";
		stream << particle_system.GetItem(6) << " ";
		stream << particle_system.GetItem(7) << " ";
		stream << particle_system.GetItem(8) << " ";

		for (int i = 0; i < 5; i += 2)
		{
			CPyObject key1 = particle_system.GetItem(i + 9);
			CPyObject key2 = particle_system.GetItem(i + 10);
			
			stream << key1.GetItem(0) << " ";
			stream << key1.GetItem(1) << " ";
			stream << key2.GetItem(0) << " ";
			stream << key2.GetItem(1) << " ";
		}

		CPyObject emit_box_size = particle_system.GetItem(19);
		
		stream << emit_box_size.GetItem(0) << " ";
		stream << emit_box_size.GetItem(1) << " ";
		stream << emit_box_size.GetItem(2) << " ";

		CPyObject emit_velocity = particle_system.GetItem(20);
		
		stream << emit_velocity.GetItem(0) << " ";
		stream << emit_velocity.GetItem(1) << " ";
		stream << emit_velocity.GetItem(2) << " ";
		stream << particle_system.GetItem(21) << " ";

		if (particle_system.Len() > 22)
			stream << particle_system.GetItem(22) << " ";
		else
			stream << "0.0 ";

		if (particle_system.Len() > 23)
			stream << particle_system.GetItem(23) << " ";
		else
			stream << "0.0 ";

		stream << std::endl;
	}
}

void ModuleSystem::WriteParties()
{
	std::ofstream stream(m_path + "parties.txt");
	CPyIter iter = m_parties.GetIter();
	int i = 0;

	stream << "partiesfile version 1" << std::endl;
	stream << m_parties.Size() << std::endl;
	stream << m_parties.Size() << std::endl;

	while (iter.HasNext())
	{
		CPyObject party = iter.Next();

		stream << 1 << " " << i << " " << i++ << " ";
		stream << "p_" << encode_id(party.GetItem(0).AsString()) << " ";
		stream << encode_str(party.GetItem(1).AsString()) << " ";
		stream << party.GetItem(2) << " ";
		stream << GetId(party.GetItem(3)) << " "; // TODO: restrict to menu, etc...
		stream << party.GetItem(4) << " ";
		stream << party.GetItem(5) << " ";
		stream << party.GetItem(6) << " ";
		stream << party.GetItem(6) << " ";
		stream << party.GetItem(7) << " ";
		stream << GetId(party.GetItem(8)) << " ";

		CPyObject position = party.GetItem(9);
		
		stream << position.GetItem(0) << " ";
		stream << position.GetItem(1) << " ";
		stream << position.GetItem(0) << " ";
		stream << position.GetItem(1) << " ";
		stream << position.GetItem(0) << " ";
		stream << position.GetItem(1) << " ";
		stream << "0.0 ";

		CPyObject members = party.GetItem(10);
		CPyIter member_iter = members.GetIter();

		stream << members.Len() << " ";

		while (member_iter.HasNext())
		{
			CPyObject member = member_iter.Next();
			
			stream << member.GetItem(0) << " ";
			stream << member.GetItem(1) << " ";
			stream << "0 ";
			stream << member.GetItem(2) << " ";
		}

		if (party.Len() > 11)
			stream << std::setprecision(7) << (3.1415926 / 180.0) * party.GetItem(11).AsFloat() << " ";
		else
			stream << "0.0 ";

		stream << std::endl;
	}
}

void ModuleSystem::WritePartyTemplates()
{
	std::ofstream stream(m_path + "party_templates.txt");
	CPyIter iter = m_party_templates.GetIter();

	stream << "partytemplatesfile version 1" << std::endl;
	stream << m_party_templates.Size() << std::endl;

	while (iter.HasNext())
	{
		CPyObject party_template = iter.Next();

		stream << "pt_" << encode_id(party_template.GetItem(0).AsString()) << " ";
		stream << encode_str(party_template.GetItem(1).AsString()) << " ";
		stream << party_template.GetItem(2) << " ";
		stream << party_template.GetItem(3) << " ";
		stream << party_template.GetItem(4) << " ";
		stream << party_template.GetItem(5) << " ";

		CPyObject members = party_template.GetItem(6);
		int num_members = members.Len();

		if (num_members > 6) // TODO: print where
		{
			Warning("party template member count exceeds 6");
			num_members = 6;
		}

		for (int i = 0; i < num_members; ++i)
		{
			CPyObject member = members.GetItem(i);
			
			stream << member.GetItem(0) << " ";
			stream << member.GetItem(1) << " ";
			stream << member.GetItem(2) << " ";

			if (member.Len() > 3)
				stream << member.GetItem(3) << " ";
			else
				stream << "0 ";
		}

		for (int i = num_members; i < 6; ++i)
		{
			stream << "-1 ";
		}

		stream << std::endl;
	}
}

void ModuleSystem::WritePostEffects()
{
	std::ofstream stream(m_path + "postfx.txt");
	CPyIter iter = m_postfx.GetIter();

	stream << "postfx_paramsfile version 1" << std::endl;
	stream << m_postfx.Size() << std::endl;

	while (iter.HasNext())
	{
		CPyObject effect = iter.Next();
		
		stream << "pfx_" << encode_id(effect.GetItem(0).AsString()) << " ";
		stream << effect.GetItem(1) << " ";
		stream << effect.GetItem(2) << " ";

		for (int i = 0; i < 3; ++i)
		{
			CPyObject params = effect.GetItem(i + 3);
			
			stream << params.GetItem(0) << " ";
			stream << params.GetItem(1) << " ";
			stream << params.GetItem(2) << " ";
			stream << params.GetItem(3) << " ";
		}

		stream << std::endl;
	}
}

void ModuleSystem::WritePresentations()
{
	std::ofstream stream(m_path + "presentations.txt");
	CPyIter iter = m_presentations.GetIter();

	stream << "presentationsfile version 1" << std::endl;
	stream << m_presentations.Size() << std::endl;

	while (iter.HasNext())
	{
		CPyObject presentation = iter.Next();
		
		stream << "prsnt_" << encode_id(presentation.GetItem(0).AsString()) << " ";
		stream << presentation.GetItem(1) << " ";
		stream << presentation.GetItem(2) << " ";
		WriteSimpleTriggerBlock(presentation.GetItem(3), stream);
		stream << std::endl;
	}
}

void ModuleSystem::WriteQuests()
{
	std::ofstream stream(m_path + "quests.txt");
	CPyIter iter = m_quests.GetIter();

	stream << "questsfile version 1" << std::endl;
	stream << m_quests.Size() << std::endl;

	while (iter.HasNext())
	{
		CPyObject quest = iter.Next();
		
		stream << "qst_" << encode_id(quest.GetItem(0).AsString()) << " ";
		stream << encode_str(quest.GetItem(1).AsString()) << " ";
		stream << quest.GetItem(2) << " ";
		stream << encode_str(quest.GetItem(3).AsString()) << " ";
		stream << std::endl;
	}
}

void ModuleSystem::WriteQuickStrings()
{
	std::ofstream stream(m_path + "quick_strings.txt");
	std::vector<std::string> quick_strings(m_quick_strings.size());
	std::map<std::string, QuickString>::const_iterator it;

	for (it = m_quick_strings.begin(); it != m_quick_strings.end(); ++it)
	{
		quick_strings[it->second.index] = it->first;
	}

	stream << quick_strings.size() << std::endl;

	for (size_t i = 0; i < quick_strings.size(); ++i)
	{
		stream << quick_strings[i] << " ";
		stream << m_quick_strings[quick_strings[i]].value << std::endl;
	}
}

void ModuleSystem::WriteSceneProps()
{
	CPyModule header_items("header_scene_props");
	CPyObject get_spr_hit_points = header_items.GetAttr("get_spr_hit_points");

	std::ofstream stream(m_path + "scene_props.txt");
	CPyIter iter = m_scene_props.GetIter();

	stream << "scene_propsfile version 1" << std::endl;
	stream << m_scene_props.Size() << std::endl;

	while (iter.HasNext())
	{
		CPyObject scene_prop = iter.Next();

		stream << "spr_" << encode_id(scene_prop.GetItem(0).AsString()) << " ";
		stream << scene_prop.GetItem(1) << " ";

		CPyTuple args(1);

		args.SetItem(0, scene_prop.GetItem(1));
		stream << get_spr_hit_points.Call(args) << " ";
		stream << encode_str(scene_prop.GetItem(2).Str()) << " ";
		stream << encode_str(scene_prop.GetItem(3).Str()) << " ";
		stream << std::endl;
		WriteSimpleTriggerBlock(scene_prop.GetItem(4), stream);
		stream << std::endl;
	}
}

void ModuleSystem::WriteScenes()
{
	std::ofstream stream(m_path + "scenes.txt");
	CPyIter iter = m_scenes.GetIter();

	stream << "scenesfile version 1" << std::endl;
	stream << m_scenes.Size() << std::endl;

	while (iter.HasNext())
	{
		CPyObject scene = iter.Next();

		stream << "scn_" << encode_id(scene.GetItem(0).AsString()) << " ";
		stream << encode_str(scene.GetItem(0).AsString()) << " ";
		stream << scene.GetItem(1) << " ";
		stream << scene.GetItem(2) << " ";
		stream << scene.GetItem(3) << " ";
		stream << scene.GetItem(4).GetItem(0) << " ";
		stream << scene.GetItem(4).GetItem(1) << " ";
		stream << scene.GetItem(5).GetItem(0) << " ";
		stream << scene.GetItem(5).GetItem(1) << " ";
		stream << scene.GetItem(6) << " ";
		stream << scene.GetItem(7) << " ";

		CPyObject passages = scene.GetItem(8);
		CPyIter passage_iter = passages.GetIter();

		stream << passages.Len() << " ";

		while (passage_iter.HasNext())
		{
			CPyObject passage = passage_iter.Next();
			int scene_id;

			if (passage.IsLong())
			{
				scene_id = passage.AsLong();
			}
			else
			{
				std::string name = passage.AsString();


				if (name.empty())
					scene_id = 0;
				else if (name == "exit")
					scene_id = 100000;
				else
					scene_id = m_ids["scn"][name];
			}
			
			stream << scene_id << " ";
		}

		CPyObject chests = scene.GetItem(9);
		CPyIter chest_iter = chests.GetIter();

		stream << chests.Len() << " ";

		while (chest_iter.HasNext())
		{
			CPyObject chest = chest_iter.Next();
			int troop_id;

			if (chest.IsLong())
				troop_id = chest.AsLong();
			else
				troop_id = m_ids["trp"][chest.AsString()];
			
			stream << troop_id << " ";
		}

		if (scene.Len() > 10)
			stream << scene.GetItem(10) << " ";
		else
			stream << "0 ";

		stream << std::endl;
	}
}

void ModuleSystem::WriteScripts()
{
	std::ofstream stream(m_path + "scripts.txt");
	CPyIter iter = m_scripts.GetIter();
	int i = 0;

	stream << "scriptsfile version 1" << std::endl;
	stream << m_scripts.Size() << std::endl;

	while (iter.HasNext())
	{
		CPyObject script = iter.Next();
		std::string name = script.GetItem(0).AsString();

		if ((m_flags & msf_obfuscate_scripts) && name.substr(0, 5) != "game_")
			stream << "script_" << i++ << " ";
		else
			stream << encode_id(name) << " ";

		WriteStatementBlock(script.GetItem(1), stream);
		stream << std::endl;
	}
}

void ModuleSystem::WriteSimpleTriggers()
{
	std::ofstream stream(m_path + "simple_triggers.txt");

	stream << "simple_triggers_file version 1" << std::endl;
	WriteSimpleTriggerBlock(m_simple_triggers, stream);
}

void ModuleSystem::WriteSkills()
{
	std::ofstream stream(m_path + "skills.txt");
	CPyIter iter = m_skills.GetIter();

	stream << m_skills.Size() << std::endl;

	while (iter.HasNext())
	{
		CPyObject skill = iter.Next();
		
		stream << "skl_" << encode_id(skill.GetItem(0).AsString()) << " ";
		stream << encode_str(skill.GetItem(1).AsString()) << " ";
		stream << skill.GetItem(2) << " ";
		stream << skill.GetItem(3) << " ";
		stream << encode_str(skill.GetItem(4).AsString()) << " ";
		stream << std::endl;
	}
}

void ModuleSystem::WriteSkins()
{
	std::ofstream stream(m_path + "skins.txt");
	int num_skins = m_skins.Size();

	if (num_skins > 16)
	{
		Warning("skin count exceeds 6");
		num_skins = 16;
	}

	stream << "skins_file version 1" << std::endl;
	stream << num_skins << std::endl;

	for (int i = 0; i < num_skins; ++i)
	{
		CPyObject skin = m_skins.GetItem(i);
		
		stream << encode_id(skin.GetItem(0).AsString()) << " ";
		stream << skin.GetItem(1) << " ";
		stream << skin.GetItem(2) << " ";
		stream << skin.GetItem(3) << " ";
		stream << skin.GetItem(4) << " ";
		stream << skin.GetItem(5) << " ";

		CPyObject face_keys = skin.GetItem(6);
		CPyIter face_key_iter = face_keys.GetIter();

		stream << face_keys.Len() << " ";

		while (face_key_iter.HasNext())
		{
			CPyObject face_key = face_key_iter.Next();

			stream << "skinkey_" << encode_id(face_key.GetItem(4).AsString()) << " ";
			stream << skin.GetItem(0) << " ";
			stream << skin.GetItem(1) << " ";
			stream << skin.GetItem(2) << " ";
			stream << skin.GetItem(3) << " ";
			stream << encode_str(face_key.GetItem(4).AsString()) << " ";
		}

		CPyObject hair_meshes = skin.GetItem(7);
		CPyIter hair_mesh_iter = hair_meshes.GetIter();

		stream << hair_meshes.Len() << " ";

		while (hair_mesh_iter.HasNext())
		{
			stream << hair_mesh_iter.Next() << " ";
		}

		CPyObject beard_meshes = skin.GetItem(8);
		CPyIter beard_mesh_iter = beard_meshes.GetIter();

		stream << beard_meshes.Len() << " ";

		while (beard_mesh_iter.HasNext())
		{
			stream << beard_mesh_iter.Next() << " ";
		}

		CPyObject hair_textures = skin.GetItem(9);
		CPyIter hair_texture_iter = hair_textures.GetIter();

		stream << hair_textures.Len() << " ";

		while (hair_texture_iter.HasNext())
		{
			stream << hair_texture_iter.Next() << " ";
		}

		CPyObject beard_textures = skin.GetItem(10);
		CPyIter beard_texture_iter = beard_textures.GetIter();

		stream << beard_textures.Len() << " ";

		while (beard_texture_iter.HasNext())
		{
			stream << beard_texture_iter.Next() << " ";
		}

		CPyObject face_textures = skin.GetItem(11);
		CPyIter face_texture_iter = face_textures.GetIter();

		stream << face_textures.Len() << " ";

		while (face_texture_iter.HasNext())
		{
			CPyObject face_texture = face_texture_iter.Next();
			
			stream << face_texture.GetItem(0) << " ";
			stream << face_texture.GetItem(1) << " ";
			
			CPyObject hair_materials;
			CPyObject hair_colors;
			int num_hair_materials = 0;
			int num_hair_colors = 0;

			if (face_texture.Len() > 2)
			{
				hair_materials = face_texture.GetItem(2);
				num_hair_materials = hair_materials.Len();

				if (face_texture.Len() > 3)
				{
					hair_colors = face_texture.GetItem(3);
					num_hair_colors = hair_colors.Len();
				}
			}

			stream << num_hair_materials << " ";
			stream << num_hair_colors << " ";

			for (int i = 0; i < num_hair_materials; ++i)
			{
				stream << encode_str(hair_materials.GetItem(i).Str()) << " ";
			}

			for (int i = 0; i < num_hair_colors; ++i)
			{
				stream << hair_colors.GetItem(i) << " ";
			}
		}

		CPyObject voices = skin.GetItem(12);
		CPyIter voice_iter = voices.GetIter();

		stream << voices.Len() << " ";

		while (voice_iter.HasNext())
		{
			CPyObject voice = voice_iter.Next();

			stream << voice.GetItem(0) << " ";
			stream << encode_id(voice.GetItem(1).Str()) << " ";
		}

		stream << skin.GetItem(13) << " ";
		stream << skin.GetItem(14) << " ";

		if (skin.Len() > 15)
			stream << GetId(skin.GetItem(15)) << " ";
		else
			stream << "0 ";

		if (skin.Len() > 16)
			stream << GetId(skin.GetItem(16)) << " ";
		else
			stream << "0 ";

		if (skin.Len() > 17)
		{
			CPyObject constraints = skin.GetItem(17);
			CPyIter constraint_iter = constraints.GetIter();

			stream << constraints.Len() << " ";

			while (constraint_iter.HasNext())
			{
				CPyObject constraint = constraint_iter.Next();
				
				stream << constraint.GetItem(0) << " ";
				stream << constraint.GetItem(1) << " ";

				int num_pairs = constraint.Len() - 2;

				stream << num_pairs << " ";

				for (int i = 0; i < num_pairs; ++i)
				{
					CPyObject pair = constraint.GetItem(i + 2);

					stream << pair.GetItem(0) << " ";
					stream << pair.GetItem(1) << " ";
				}
			}
		}
		else
		{
			stream << "0 ";
		}

		stream << std::endl;
	}
}

void ModuleSystem::WriteSounds()
{
	std::ofstream stream(m_path + "sounds.txt");
	CPyIter iter = m_sounds.GetIter();
	std::map<std::string, unsigned long> samples;
	std::vector<std::string> samples_vec;

	while (iter.HasNext())
	{
		CPyObject sound = iter.Next();
		CPyObject sound_files = sound.GetItem(2);
		CPyIter sound_file_iter = sound_files.GetIter();

		while (sound_file_iter.HasNext())
		{
			CPyObject sound_file = sound_file_iter.Next();
			std::string file = sound_file.GetItem(0).AsString();

			if (samples.find(file) == samples.end())
			{
				samples[file] = sound_file.GetItem(1).AsLong();
				samples_vec.push_back(file);
			}
		}
	}

	stream << "soundsfile version 3" << std::endl;
	stream << samples_vec.size() << std::endl;

	for (int i = 0; i < samples_vec.size(); ++i)
	{
		stream << samples_vec[i] << " ";
		stream << samples[samples_vec[i]] << " ";
	}

	iter = m_sounds.GetIter();

	while (iter.HasNext())
	{
		CPyObject sound = iter.Next();
		
		stream << "snd_" << encode_id(sound.GetItem(0).AsString()) << " ";
		stream << sound.GetItem(1) << " ";
		stream << sound.GetItem(2) << " ";

		CPyObject sound_files = sound.GetItem(2);
		CPyIter sound_file_iter = sound_files.GetIter();

		while (sound_file_iter.HasNext())
		{
			CPyObject sound_file = sound_file_iter.Next();
			std::string file = sound_file.GetItem(0).AsString();

			if (samples.find(file) == samples.end())
			{
				samples[file] = sound_file.GetItem(1).AsLong();
				samples_vec.push_back(file);
			}
		}
		stream << std::endl;
	}
}

void ModuleSystem::WriteStrings()
{
	std::ofstream stream(m_path + "strings.txt");
	CPyIter iter = m_strings.GetIter();
	
	stream << "stringsfile version 1" << std::endl;
	stream << m_strings.Size() << std::endl;

	while (iter.HasNext())
	{
		CPyObject string = iter.Next();
		
		stream << "str_" << encode_id(string.GetItem(0).AsString()) << " ";
		stream << encode_str(string.GetItem(1).AsString()) << " ";
		stream << std::endl;
	}
}

void ModuleSystem::WriteTableaus()
{
	std::ofstream stream(m_path + "tableau_materials.txt");
	CPyIter iter = m_tableau_materials.GetIter();

	stream << m_tableau_materials.Size() << std::endl;

	while (iter.HasNext())
	{
		CPyObject tableau = iter.Next();

		stream << "tab_" << encode_id(tableau.GetItem(0).AsString());
		stream << tableau.GetItem(1) << " ";
		stream << encode_str(tableau.GetItem(2).AsString()) << " ";
		stream << tableau.GetItem(3) << " ";
		stream << tableau.GetItem(4) << " ";
		stream << tableau.GetItem(5) << " ";
		stream << tableau.GetItem(6) << " ";
		stream << tableau.GetItem(7) << " ";
		stream << tableau.GetItem(8) << " ";
		WriteStatementBlock(tableau.GetItem(9), stream);
		stream << std::endl;
	}
}

void ModuleSystem::WriteTriggers()
{
	std::ofstream stream(m_path + "triggers.txt");

	stream << "triggersfile version 1" << std::endl;
	WriteTriggerBlock(m_triggers, stream);
}

void ModuleSystem::WriteTroops()
{
	std::ofstream stream(m_path + "troops.txt");
	CPyIter iter = m_troops.GetIter();

	stream << "troopsfile version 2" << std::endl;
	stream << m_troops.Size() << std::endl;

	while (iter.HasNext())
	{
		CPyObject troop = iter.Next();
		
		stream << "trp_" << encode_id(troop.GetItem(0).AsString()) << " ";
		stream << encode_str(troop.GetItem(1).AsString()) << " ";
		stream << encode_str(troop.GetItem(2).AsString()) << " ";

		if (troop.Len() > 13)
			stream << encode_str(troop.GetItem(13).Str()) << " ";
		else
			stream << "0 ";

		stream << troop.GetItem(3) << " ";
		stream << troop.GetItem(4) << " ";
		stream << troop.GetItem(5) << " ";
		stream << troop.GetItem(6) << " ";

		if (troop.Len() > 14)
			stream << troop.GetItem(14) << " ";
		else
			stream << "0 ";

		if (troop.Len() > 15)
			stream << troop.GetItem(15) << " ";
		else
			stream << "0 ";

		CPyObject items = troop.GetItem(7);
		int num_items = items.Len();

		stream << num_items << " ";

		for (int i = 0; i < num_items; ++i)
		{
			stream << GetId(items.GetItem(i)) << " 0 ";
		}

		for (int i = num_items; i < 64; ++i)
		{
			stream << "-1 0 ";
		}

		CPyNumber attribs = troop.GetItem(8);
		
		stream << ((attribs >> CPyNumber(0)) & 0xFF) << " ";
		stream << ((attribs >> CPyNumber(8)) & 0xFF) << " ";
		stream << ((attribs >> CPyNumber(16)) & 0xFF) << " ";
		stream << ((attribs >> CPyNumber(24)) & 0xFF) << " ";
		stream << ((attribs >> CPyNumber(32)) & 0xFF) << " ";

		CPyNumber proficiencies = troop.GetItem(9).AsNumber();

		for (int i = 0; i < 7; ++i)
		{
			stream << (proficiencies & 0x3FF) << " ";
			proficiencies = proficiencies >> CPyNumber(10);
		}

		CPyNumber skills = troop.GetItem(10).AsNumber();

		for (int i = 0; i < 6; ++i)
		{
			stream << ((skills >> CPyNumber(i * 32)) & 0xFFFFFFFF) << " ";
		}

		for (int i = 0; i < 2; ++i)
		{
			CPyNumber face_key;
			
			if (troop.Len() > i + 11)
				face_key = troop.GetItem(i + 11).AsNumber();

			for (int j = 0; j < 4; ++j)
			{
				stream << ((face_key >> CPyNumber((3 - j) * 64)) & 0xFFFFFFFFFFFFFFFF) << " ";
			}
		}

		stream << std::endl;
	}
}

void ModuleSystem::WriteSimpleTriggerBlock(const CPyObject &simple_trigger_block, std::ostream &stream)
{
	CPyIter simple_trigger_iter = simple_trigger_block.GetIter();
	stream << simple_trigger_block.Len() << std::endl;

	while (simple_trigger_iter.HasNext())
	{
		WriteSimpleTrigger(simple_trigger_iter.Next(), stream);
		stream << std::endl;
	}
}

void ModuleSystem::WriteSimpleTrigger(const CPyObject &simple_trigger, std::ostream &stream)
{
	stream << simple_trigger.GetItem(0) << " ";
	WriteStatementBlock(simple_trigger.GetItem(1), stream);
}

void ModuleSystem::WriteTriggerBlock(const CPyObject &trigger_block, std::ostream &stream)
{
	CPyIter trigger_iter = trigger_block.GetIter();

	stream << trigger_block.Len() << std::endl;

	while (trigger_iter.HasNext())
	{
		WriteTrigger(trigger_iter.Next(), stream);
		stream << std::endl;
	}
}

void ModuleSystem::WriteTrigger(const CPyObject &trigger, std::ostream &stream)
{
	stream << trigger.GetItem(0) << " ";
	stream << trigger.GetItem(1) << " ";
	stream << trigger.GetItem(2) << " ";
	WriteStatementBlock(trigger.GetItem(3), stream);
	WriteStatementBlock(trigger.GetItem(4), stream);
}

void ModuleSystem::WriteStatementBlock(const CPyObject &statement_block, std::ostream &stream)
{
	int num_statements = statement_block.Len();

	m_local_vars.clear();
	stream << num_statements << " ";

	for (int i = 0; i < num_statements; ++i)
	{
		WriteStatement(statement_block.GetItem(i), stream);
	}

	std::map<std::string, Variable>::const_iterator it;

	for (it = m_local_vars.begin(); it != m_local_vars.end(); ++it)
	{
		if (it->second.usages == 0 && it->first.substr(0, 6) != "unused")
			Warning("unused local variable :" + it->first);
	}
}

void ModuleSystem::WriteStatement(const CPyObject &statement, std::ostream &stream)
{
	if (statement.IsTuple() || statement.IsList())
	{
		long long opcode = statement.GetItem(0).AsLong();
		int num_operands = statement.Len() - 1;

		stream << opcode << " ";

		if (num_operands > 16)
		{
			Warning("operand count exceeds 16");
			num_operands = 16;
		}

		stream << num_operands << " ";

		for (int i = 0; i < num_operands; ++i)
		{
			stream << ParseOperand(statement, i + 1) << " ";
		}
	}
	else if (statement.IsLong())
	{
		stream << (long long)statement.AsLong() << " 0 ";
	}
	else
	{
		Error("unrecognized statement type " + (std::string)statement.Type().Str());
	}
}
