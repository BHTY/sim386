__declspec(dllexport) int lstrlenA(const char* lpString){
	int i = 0;
	while(*lpString){
		lpString++;
		i++;
	}
	return i;
}