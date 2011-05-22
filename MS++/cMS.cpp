#include <Windows.h>
#include "ModuleSystem.h"
#include "StringUtils.h"

int main(int argc, char **argv)
{
	std::map<std::string, std::string> options;
	std::string prev_option;

	for (int i = 1; i < argc; ++i)
	{
		std::string cur_option = trim(std::string(argv[i]));

		if (cur_option[0] == '-')
		{
			prev_option = cur_option;
			options[cur_option] = ".";
		}
		else if (!prev_option.empty())
		{
			options[prev_option] = cur_option;
			prev_option.clear();
		}
		else
		{
			options[cur_option] = ".";
		}
	}

	unsigned long long flags = 0;

	if (!options["-strict"].empty())
		flags |= msf_strict;

	if (!options["-skip-id-files"].empty())
		flags |= msf_skip_id_files;

	if (!options["-list-resources"].empty())
		flags |= msf_list_resources;

	if (!options["-hide-global-vars"].empty())
		flags |= msf_obfuscate_global_vars;

	if (!options["-hide-scripts"].empty())
		flags |= msf_obfuscate_scripts;

	if (!options["-hide-dialog-states"].empty())
		flags |= msf_obfuscate_dialog_states;

	if (!options["-hide-tags"].empty())
		flags |= msf_obfuscate_tags;
	
	char in_path[MAX_PATH];

	if (!options["-in-path"].empty())
		strncpy_s(in_path, options["-in-path"].c_str(), MAX_PATH);
	else
		GetCurrentDirectory(MAX_PATH, in_path);

	std::string out_path;

	if (!options["-out-path"].empty())
		out_path = options["-out-path"];

	ModuleSystem ms(in_path, out_path);

	ms.Compile(flags);
}
