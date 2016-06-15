/********************************************
*
*  Name: Haoyang Chen
*  Section: Wed 2:00-3:00
*  Assignment: Lab 6 - Write to LCD display
*
********************************************/

void init_lcd();
void init_acd();
void moveto(unsigned char row, unsigned char col);
void stringout(char *str);
void writecommand(unsigned char x);
void writedata(unsigned char x);