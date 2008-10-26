#ifndef STYLEPARSER_H_
#define STYLEPARSER_H_

#include <String.h>

struct style_s {
	BString	id;
	BString	value;
};

class StyleParser {

public:
					StyleParser();
					StyleParser(BString data);
					StyleParser(const char *data);

virtual				~StyleParser();

		void		SetData(BString &data);
		void		SetData(const char *data);

		void		AddData(BString &data);
		void		AddData(const char *data);

		void		Reset();

		style_s		*GetNext();
		void		Rewind();

		style_s		*Find(BString id);
		style_s		*Find(const char *id);

		bool		FindAndAsign(const char *id, const char *format, void *to_value);

private:

		BString		fData;
		BString		fBuffer;
		int			fPosition;
};

#endif // STYLEPARSER_H_
