#ifndef QCLV_H
#define QCLV_H

#include <Roster.h>
#include <DataIO.h>
#include <Mime.h>
#include <NodeMonitor.h>
#include <PopUpMenu.h>
#include <Query.h>
#include <TypeConstants.h>
#include <Volume.h>
#include <VolumeRoster.h>
#include <kernel/fs_info.h>
#include <kernel/fs_attr.h>

#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include <map>
#include <vector>

#include <common/IMKitUtilities.h>
#include <common/ColumnListView.h>
#include <common/ColumnTypes.h>

#include "SVGCache.h"
#include "MenuColumns.h"

extern const char *kFolderState;
extern const int16 kAttrIndexOffset;

extern const char *kTrackerQueryVolume;
extern const char *kTrackerQueryPredicate;
extern const char *kTrackerQueryType;
extern const char *kTrackerQueryInitMime;

typedef struct ci_internal {
	BColumn *col;
	uint32 hash;
	BString publicName;
	BString internalName;
	int32 index;
	int32 type;

	ci_internal(void) {
		this->col = NULL;
		this->hash = 0;
		this->publicName = "";
		this->internalName = "";
		this->index = 0;
		this->type = 0;
	};
	virtual ~ci_internal(void) {
		this->publicName.SetTo("");
		this->internalName.SetTo("");
	};
} col_info;

typedef struct {
	entry_ref ref;
	node_ref nref;
} result;

typedef std::map<BString, BString> attr_map;
typedef std::map<BString, uint32> type_map;
typedef std::map<BString, int32> ai_map;
typedef std::map<int32, BString> ia_map;
typedef std::map<entry_ref, BRow *> ref_map;
typedef std::map<BString, BColumn *> pc_map;

typedef std::map<uint32, col_info *> hash_info_map;
typedef std::map<BString, col_info *> internal_info_map;
typedef std::map<BString, col_info *> public_info_map;

typedef std::map<BString, BString> mime_map;

typedef std::vector<BVolume> vollist;
typedef std::vector<BQuery *> querylist;

typedef std::map<entry_ref, result> resultmap;
typedef std::vector<entry_ref> pending_stack;


enum {
	qclvAddRow = 'qc01',
	qclvQueryUpdated = 'qc02',
	qclvRequestPending = 'qc03',
	qclvInvoke = 'qc04',
};

class QueryColumnListView : public BColumnListView {
	public:
							QueryColumnListView(BRect rect,
						                const char *name,
						                uint32 resizingMode,
										uint32 drawFlags,
										entry_ref ref,
										BMessage *msg = NULL,
										BMessenger *notify = NULL,
										border_style border = B_NO_BORDER,
										bool showHorizontalScrollbar = true);
							~QueryColumnListView();
							
		virtual status_t	AddRowByRef(entry_ref *ref);
		virtual status_t	RemoveRowByRef(entry_ref *ref);
		virtual status_t	RefForRow(BRow *row, entry_ref *ref);
				
			virtual void	MessageReceived(BMessage *msg);
			virtual void 	KeyDown(const char *bytes, int32 numBytes);
			
			virtual void	AttachedToWindow(void);
			
			 		char	*MIMETypeFor(entry_ref *ref);
			const char		*MIMEType(void);
			const char		*Name(void);
	private:
				status_t	ExtractColumnState(BMallocIO *buffer);
				status_t	ExtractViewState(BMallocIO *buffer);
				status_t	AddStatColumns(void);
				status_t	AddMIMEColumns(BMessage *msg);
			static int32	BackgroundAdder(void *arg);
				uint32		CalculateHash(const char *name, uint32 type);
				void		AddColInfo(col_info *column);
				col_info	*MakeColInfo(BColumn *col, uint32 hash,
								const char *publicName, const char *internalName,
								uint32 type);
			
				status_t	ExtractPredicate(entry_ref *ref, char **buffer,
								int32 *length);
				status_t	ExtractVolumes(entry_ref *ref, vollist *vols);
				entry_ref	ActionFor(entry_ref *ref);
			
		
				BMessage	*fMessage;
				BMessenger	*fNotify;
		
				entry_ref	fRef;
				BString		fMIMEString;
				attr_map	fAttributes;
				type_map	fAttrTypes;
				ai_map		fAttrIndex;
				ia_map		fIndexAttr;
				
			hash_info_map	fHashCols;
		internal_info_map	fInternalCols;
			public_info_map	fPublicCols;
				int32		fAttrIndexOffset;

				ref_map		fRefRows;
				
				thread_id	fThreadID;
				BMessenger	*fSelfMsgr;
			pending_stack	fPending;

//				Query stuff
				bool		fInitialFetch;
				BString		fPredicate;
				vollist		fVolumes;
				querylist	fQueries;
				resultmap	fResults;
			static mime_map	fMimeTypes;
				
};

#endif
