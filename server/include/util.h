void insertLoggedUser(char* username);
void writeLoginOnFile(char* username, char* record, int len, int porta, time_t rawtime);
bool searchUser(char* user_psw);
void readCredentials(char* username, char* password);
void clientDisconnection(int sock);
void restoreLogin();
void restore();