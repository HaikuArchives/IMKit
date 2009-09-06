#ifndef IM_CONTACT_H
#define IM_CONTACT_H

#include <Bitmap.h>
#include <Entry.h>
#include <List.h>
#include <String.h>

#include <kernel/fs_info.h>
#include <kernel/fs_attr.h>

namespace IM {

class Contact
{
	public:
		Contact();
		Contact( const entry_ref & );
		Contact( const BEntry & );
//		Contact( const node_ref & );
		Contact( const Contact & );
		~Contact();
		
		// 
		void SetTo( const entry_ref &);
		void SetTo( const BEntry &);
//		void SetTo( const node_ref &);
		void SetTo( const Contact &);
		
		// returns B_OK if the Contact is connected to a valid People-file
		status_t InitCheck();
		
		bool Exists();
		
		// call this function to reload information from attributes
		void Update();
		
		// Connection management
		status_t AddConnection( const char * proto_id );
		status_t RemoveConnection( const char * proto_id );
		int CountConnections();
		status_t ConnectionAt( int index, char * );
		status_t FindConnection( const char * protocol, char * );
		
		// Various data
		status_t	GetStatus(char *bug, int size);
		status_t	SetStatus(const char *);
		status_t	GetName(char *buf, int size);
		status_t	SetName(const char *);
		status_t	GetNickname(char *buf, int size);
		status_t	SetNickname(const char *);
		status_t	GetEmail(char *buf, int size);
		status_t	SetEmail(const char *);
		status_t	GetURL(char* buf, int size);
		status_t	SetURL(const char*);
		status_t	GetBirthday(char* buf, int size);
		status_t	SetBirthday(const char*);

		int32		CountGroups(void);
		const char	*GroupAt(int32 index);
		
		status_t	SetBuddyIcon(const char *protocol, BBitmap *icon);
		BBitmap		*GetBuddyIcon(const char *protocol, int16 size = 48);
		
		// comparison, both with Contacts and entry_refs and node_refs
		// representing Contacts
		bool operator == ( const entry_ref & ) const;
		bool operator == ( const BEntry & ) const;
		bool operator == ( const Contact & ) const;
		bool operator < ( const Contact & ) const;
		
		entry_ref	EntryRef(void) const;
		
		// for easy addition to BMessages
		operator const entry_ref * () const;
//		const node_ref operator Contact();
		
	private:
		status_t	LoadConnections();
		status_t	SaveConnections();
		status_t	ReadAttribute(const char * attr, char * buffer, int bufsize);
		status_t	ReadAttribute(const char *attr, char **buffer, int32 *size);
		status_t	WriteAttribute(const char *name, const char *buffer, int32 size);
		status_t	LoadGroups(void);
		
		void		Clear();
		
		entry_ref	fEntry;
		BList		fConnections;
		BList		fGroups;
		
};

};

#endif
