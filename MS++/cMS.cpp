#include "ModuleSystem.h"
#if defined _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#define MAX_PATH 1024
#define strncpy_s strncpy
#endif
#include "StringUtils.h"

int main(int argc, char **argv)
{
	std::map<std::string, std::string> options;
	std::string prev_option;

	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];
		std::string cur_option = trim(arg);

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
	
	options.erase("-strict");

	if (!options["-skip-id-files"].empty())
		flags |= msf_skip_id_files;
	
	options.erase("-skip-id-files");

	if (!options["-list-resources"].empty())
		flags |= msf_list_resources;
	
	options.erase("-list-resources");

	if (!options["-hide-global-vars"].empty())
		flags |= msf_obfuscate_global_vars;
	
	options.erase("-hide-global-vars");

	if (!options["-hide-scripts"].empty())
		flags |= msf_obfuscate_scripts;
	
	options.erase("-hide-scripts");

	if (!options["-hide-dialog-states"].empty())
		flags |= msf_obfuscate_dialog_states;
	
	options.erase("-hide-dialog-states");

	if (!options["-hide-tags"].empty())
		flags |= msf_obfuscate_tags;
	
	options.erase("-hide-tags");
	
	char in_path[MAX_PATH];

	if (!options["-in-path"].empty())
		strncpy_s(in_path, options["-in-path"].c_str(), MAX_PATH);
	else
#if defined _WIN32
		if (!GetCurrentDirectory(MAX_PATH, in_path))
#else
		if (!getcwd(in_path, MAX_PATH))
#endif
		std::cout << "Error getting current directory.";

	options.erase("-in-path");

	std::string out_path;

	if (!options["-out-path"].empty())
		out_path = options["-out-path"];

	options.erase("-out-path");

	std::map<std::string, std::string>::const_iterator it;

	for (it = options.begin(); it != options.end(); ++it)
	{
		std::cout << "Unrecognized option: " << it->first;
		
		if (it->second != ".")
			std::cout << " " << it->second;
		
		std::cout << std::endl;
	}

	ModuleSystem ms(in_path, out_path);

	ms.Compile(flags);
}
