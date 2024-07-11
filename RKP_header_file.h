#ifndef RKP_header_file
#define RKP_header_file
int Commands(int* send_mode, int* file_mode, int argc, char *argv[]);
int Measurement(int **Values);
void BMPcreator(int *Values, int NumValues);
int FindPID();
void SendViaFile(int *Values, int NumValues);
void ReceiveViaFile();
void SendViaSocket(int *Values, int NumValues);
void ReceiveViaSocket();
void SignalHandler(int sig);
#endif