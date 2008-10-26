#ifndef ZOIDBERG_PEOPLE_WINDOW_H
#define ZOIDBERG_PEOPLE_WINDOW_H

#include <Window.h>
#include <Node.h>

class BView;

class PeopleWindow : public BWindow {
	public:
		PeopleWindow(entry_ref *);
		
		bool QuitRequested();
		
	private:
		BNode node;
		BView *fTopView;
};

#endif // ZOIDBERG_PEOPLE_WINDOW_H
