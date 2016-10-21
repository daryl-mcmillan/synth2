typedef void (*serialHandler)(unsigned char);
void serialSetupHandler( int baud, serialHandler newHandler );
void serialSetup( int baud );
void serialWrite( const char c );
void serialWrite( const char* str );
void serialWrite( unsigned int number );
unsigned char serialRead();
int serialReadReady();
