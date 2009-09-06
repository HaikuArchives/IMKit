#include <ctype.h>

#include "htmlparse.h"

void encode_html( BString &msg ) {
	// BStrings are slow and sucky, but this is real easy
	msg.ReplaceAll("&","&amp;");
	msg.ReplaceAll("\"","&quot;");
	msg.ReplaceAll("<","&lt;");
	msg.ReplaceAll(">","&gt;");
	
	msg.ReplaceAll("[b]","<b>");
	msg.ReplaceAll("[/b]","</b>");
	msg.ReplaceAll("[i]","<i>");
	msg.ReplaceAll("[/i]","</i>");
	
	msg.Prepend("<html><body>");
	msg.Append("</body></html>");
}

void parse_html( char * msg )
{
	bool is_in_tag = false;
	int copy_pos = 0;
	
	char * copy = new char[strlen(msg)+1];
	
	for ( int i=0; msg[i]; i++ )
	{
		switch ( msg[i] )
		{
			case '<':
				is_in_tag = true;
				for (int j = i+1; msg[j]; j++) {
					if (isspace(msg[j])) continue;
					else if (tolower(msg[j]) == 'a') {
						copy[copy_pos++] = '[';
						copy[copy_pos++] = ' ';
						for (; msg[j] && msg[j] != '=' /* This is horrible */; j++); j++;
						for (; msg[j] && isspace(msg[j]); j++);
						if (msg[j] == '\"') j++;
						for (; msg[j] && !isspace(msg[j]) && msg[j] != '\"'; j++)
								copy[copy_pos++] = msg[j];
						copy[copy_pos++] = ' ';
						copy[copy_pos++] = ']';
						copy[copy_pos++] = ' ';
					} else break;
				}
							
				break;
			case '>':
				is_in_tag = false;
				break;
			case '&':
				if (strncmp("&quot;",&msg[i],6) == 0) {
					copy[copy_pos++] = '\"';
					i += 5;
					break;
				}
				if (strncmp("&lt;",&msg[i],4) == 0) {
					copy[copy_pos++] = '<';
					i += 3;
					break;
				}
				if (strncmp("&gt;",&msg[i],4) == 0) {
					copy[copy_pos++] = '>';
					i += 3;
					break;
				}
				if (strncmp("&amp;",&msg[i],5) == 0) {
					copy[copy_pos++] = '&';
					i += 4;
					break;
				}
			default:
				if ( !is_in_tag )
				{
					copy[copy_pos++] = msg[i];
				}
		}
	}
	
	copy[copy_pos] = 0;
	
	strcpy(msg, copy);
	
	delete copy;
}
