#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <inp> <cmd> <out>\n", argv[0]);
        return 1;
    }

    char *inp = argv[1];
    char *cmd = argv[2];
    char *out = argv[3];
    
    FILE *fin = fopen(inp, "r");
    if (fin == NULL) {
        fprintf(stderr, "Error: cannot open file %s\n", inp);
        return 1;
    }

    FILE *fout = fopen(out, "w");
    if (fout == NULL) {
        fprintf(stderr, "Error: cannot open file %s\n", out);
        fclose(fin);
        return 1;
    }

    size_t cmdline_size = strlen(cmd) + strlen(inp) + strlen(out) + 3;
    char *cmdline = (char *)malloc(cmdline_size);

    snprintf(cmdline, cmdline_size, "%s %s %s", cmd, inp, out);

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    HANDLE hRead, hWrite;
    SECURITY_ATTRIBUTES sa;
    char buf[4096];
    DWORD read;

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
        fprintf(stderr, "Error: cannot create pipe\n");
        return 1;
    }

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdOutput = hWrite;
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdOutput = hWrite;
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcess(NULL, cmdline, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        fprintf(stderr, "Error: cannot create process\n");
        free(cmdline);
        fclose(fin);
        fclose(fout);
        return 1;
    }
    CloseHandle(hWrite);

    while (ReadFile(hRead, buf, sizeof(buf), &read, NULL) && read > 0) {
        fwrite(buf, 1, read, fout);
    }

    CloseHandle(hRead);

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    free(cmdline);
    fclose(fin);
    fclose(fout);

    return 0;
}