#include <Node.h>
#include <NodeInfo.h>
#include <Message.h>
#include <Messenger.h>
#include <TrackerAddOn.h>
#include <stdio.h>
#include <fs_attr.h>
#include <Alert.h>

void
merge_contacts( entry_ref _src, entry_ref _dst )
{
	BNode src(&_src), dst(&_dst);
	
	char attr_name[B_ATTR_NAME_LENGTH], data[65536];
	
	while ( src.GetNextAttrName(attr_name) == B_OK )
	{
		attr_info info;
		
		src.GetAttrInfo(attr_name,&info);
		
		if ( strcmp(attr_name,"IM:connections") == 0 )
		{ // merge connections
			char dst_data[1024];
			int32 num_read;
			
			num_read = src.ReadAttr(
				attr_name, info.type, 0,
				data, info.size
			);
			data[num_read] = 0;
			
			num_read = dst.ReadAttr(
				attr_name, info.type, 0,
				dst_data, info.size
			);
			if ( num_read > 0 )
				dst_data[num_read] = 0;
			else
				dst_data[0] = 0;
			
			// remove trailing ';'
			if ( strlen(dst_data) > 0 && dst_data[strlen(dst_data)-1] == ';' )
				dst_data[strlen(dst_data)-1] = 0;
			
			if ( strlen(data) > 0 && data[strlen(data)-1] == ';' )
				data[strlen(data)-1] = 0;
			
			char result[1024];
			result[0] = 0;
			
			if ( data[0] != 0 && dst_data[0] != 0 )
			{ // both contacts have connections
				sprintf(result,"%s;%s", data, dst_data);
			} else
			{
				if ( dst_data[0] != 0 )
				{ // only dst has connections
					strcpy(result, dst_data);
				}
				if ( data[0] != 0 )
				{ // only src has connections
					strcpy(result, data);
				}
			}
			
			if ( result[0] )
			{ // no need to write the attr if there's no data..
				dst.WriteAttr(
					attr_name, info.type, 0,
					result, strlen(result)+1
				);
			}
		} else
		{ // other attr, just copy
			src.ReadAttr(
				attr_name, info.type, 0,
				data, info.size
			);
			
			dst.WriteAttr(
				attr_name, info.type, 0,
				data, info.size
			);
		}
	}
	
	BEntry entry(&_src);
	entry.Remove();
}

void
process_refs(entry_ref /*dir*/, BMessage * msg, void * )
{
	type_code tc;
	int32 count=0;
	
	msg->GetInfo("refs",&tc,&count);
	
	if ( count != 2 )
	{
		BAlert * alert = new BAlert("IM Alert", "You must select two files to merge","Ok");
		alert->Go();
		return;
	}
	
	entry_ref refs[2];
	char name1[512], name2[512];
	
	for ( int i=0; msg->FindRef("refs",i,&refs[i]) == B_OK; i++ )
	{
		BNode node(&refs[i]);
		BNodeInfo info(&node);
		
		char type[512];
		
		if ( info.GetType(type) != B_OK )
		{ // error
			BAlert * alert = new BAlert("IM Error", "Error reading file type", "Ok");
			alert->Go();
			return;
		}
		
		if ( strcmp(type,"application/x-person") != 0 )
		{ // error
			BAlert * alert = new BAlert("IM Error", "Wrong file type", "Ok");
			alert->Go();
			return;
		}
	}
	
	BEntry entry1( &refs[0] );
	entry1.GetName( name1 );
	
	BEntry entry2( &refs[1] );
	entry2.GetName( name2 );
	
	// got file info, now ask which direction to merge
	char a_to_b[1024], b_to_a[1024];
	sprintf(a_to_b,"\"%s\" > \"%s\"", name1, name2 );
	sprintf(b_to_a,"\"%s\" > \"%s\"", name2, name1 );
	
	BAlert * alert = new BAlert(
		"IM Question",
		"Choose a merge direction. The file on the left side will be deleted.",
		a_to_b,
		b_to_a,
		"Cancel"
	);
	
	int32 selection = alert->Go();
	
	switch ( selection )
	{
		case 0:
			// a_to_b
			merge_contacts(refs[0], refs[1]);
			break;
		case 1:
			// b_to_a
			merge_contacts(refs[1], refs[0]);
			break;
		default:
			// cancel
			break;
	}
}
