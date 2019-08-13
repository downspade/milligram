
#include <windows.h>
#include <winreg.h>


/* ===================== Utility Function =========================== */
/* knRegTinyGetのバッファサイズ指定版 */
int
knRegTinyGetEx(HKEY tree, const char *key_name, const char *val_name,
		      LPVOID buffer, DWORD max_data)
{
	DWORD reg_status;
	HKEY key;
//	DWORD max_data;
	DWORD type;

	reg_status= RegOpenKeyEx(tree, key_name, 0, KEY_READ, &key);
	if (reg_status == ERROR_SUCCESS) {

//		max_data= 1024;
		reg_status= RegQueryValueEx(key, val_name, NULL,
									&type,
									(unsigned char *)buffer, &max_data);
		RegCloseKey(key);
		if (reg_status == ERROR_SUCCESS) {
			switch (type) {
			case REG_SZ: case REG_EXPAND_SZ: case REG_MULTI_SZ:
			case REG_LINK:
				((char *)buffer)[max_data]= 0;
			}
			return 1;
		}
	}

	return 0;
}

int
knRegTinyGet(HKEY tree, const char *key_name, const char *val_name,
		      LPVOID buffer)
{
	DWORD reg_status;
	HKEY key;
	DWORD max_data;
	DWORD type;

	reg_status= RegOpenKeyEx(tree, key_name, 0, KEY_READ, &key);
	if (reg_status == ERROR_SUCCESS) {

		max_data= 1024;
		reg_status= RegQueryValueEx(key, val_name, NULL,
									&type,
									(unsigned char *)buffer, &max_data);
		RegCloseKey(key);
		if (reg_status == ERROR_SUCCESS) {
			switch (type) {
			case REG_SZ: case REG_EXPAND_SZ: case REG_MULTI_SZ:
			case REG_LINK:
				((char *)buffer)[max_data]= 0;
			}
			return 1;
		}
	}

	return 0;
}

int
knRegTinySet(HKEY tree, const char *key_name, const char *val_name,
		      DWORD type, LPVOID buffer, DWORD length)
{
	DWORD reg_status;
	HKEY key;
	DWORD result;

	reg_status= RegCreateKeyEx(tree, key_name, 0, NULL, REG_OPTION_NON_VOLATILE,
								KEY_SET_VALUE, NULL, &key, &result);
	if (reg_status == ERROR_SUCCESS) {

		reg_status= RegSetValueEx(key, val_name, 0, type, (unsigned char *)buffer, length);
		RegCloseKey(key);
		if (reg_status == ERROR_SUCCESS) {
			return 1;
		}
	}

	return 0;
}
