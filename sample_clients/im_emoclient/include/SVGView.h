#ifndef SVGVIEW_H_
#define SVGVIEW_H_

#include <View.h>
#include "ObjectList.h"
#include "SVGDefs.h"

// typedef to remove dependence on expat.h
//struct XML_ParserStruct;
//typedef struct XML_ParserStruct *XML_Parser;

class BNode;
class BEntry;
class BGradient;
class BStop;
class BSVGPath;
class entry_ref;
class gradient_s;
class style_s;
class Matrix2D;

class BSVGView : public BView {

public:
						BSVGView(	BRect frame,
									const char *name,
									uint32 resizeMask,
									uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS | B_FULL_UPDATE_ON_RESIZE);
virtual					~BSVGView();

						BSVGView(BMessage *data);
static	BArchivable		*Instantiate(BMessage *data);
virtual	status_t		Archive(BMessage *data, bool deep = true) const;

		status_t		LoadFromFile(const char *name);
		status_t		LoadFromFile(BEntry *entry);
		status_t		LoadFromFile(entry_ref *ref);
		status_t		LoadFromFile(BFile *file);
		status_t		LoadFromAttribute(	const char *filename,
											const char *attribute);
		status_t		LoadFromAttribute(	BEntry *entry,
											const char *attribute);
		status_t		LoadFromAttribute(	entry_ref *ref,
											const char *attribute);
		status_t		LoadFromAttribute(	BNode *node,
											const char *attribute);
		status_t		LoadFromPositionIO(BPositionIO *positionio);
		status_t		LoadFromString(BString &string);

		status_t		ExportToFile(const char *name);
		status_t		ExportToFile(BEntry *entry);
		status_t		ExportToFile(entry_ref *ref);
		status_t		ExportToFile(BFile *file);
		status_t		ExportToAttribute(	const char *filename,
											const char *attribute);
		status_t		ExportToAttribute(	BEntry *entry,
											const char *attribute);
		status_t		ExportToAttribute(	entry_ref *ref,
											const char *attribute);
		status_t		ExportToAttribute(	BNode *node,
											const char *attribute);
		status_t		ExportToPositionIO(BPositionIO *positionio);
		status_t		ExportToString(BString &data);


virtual	void			AddPath(BSVGPath *path);
		BSVGPath		*RemovePath(BSVGPath *path);
		BSVGPath		*RemovePathAt(int index);
		BSVGPath		*PathAt(int index);
		int32			CountPaths();

virtual	void			AddGradient(BGradient *gradient);
		BGradient		*RemoveGradient(const char *name);
		BGradient		*RemoveGradient(BGradient *gradient);
		BGradient		*RemoveGradientAt(int index);
		
		BGradient		*GradientAt(int index);
		int32			CountGradients();
		
		BGradient		*FindGradient(BString name);
		BGradient		*FindGradient(const char *name);

		void			ClearPaths();
		void			ClearGradients();
		void			Clear();

virtual	void			SetScale(float scale);
		float			Scale() const;
virtual	void			SetScaleToFit(bool enabled);
		bool			ScaleToFit() const;
virtual void			SetFitContent(bool enabled);
		bool			FitContent() const;
virtual	void			SetSuperSampling(bool enabled);
		bool			SuperSampling() const;
virtual	void			SetSampleSize(uint32 samplesize);
		uint32			SampleSize() const;

		float			GraphicsWidth() const;
		float			GraphicsHeight() const;

		void			SetViewColor(rgb_color color);
		void			SetViewColor(uchar r, uchar g, uchar b, uchar a = 255);
		rgb_color		ViewColor();

virtual	void			Draw(BRect updateRect);
virtual	void			FrameResized(float new_width, float new_height);

		in_state_s		CurrentState() const;

private:

		BObjectList<BGradient>	*fGradients;
		BObjectList<BSVGPath>	*fPaths;
		int						fXMLDepth;
		bool					fXMLSkipElements;
		XML_Parser				fParser;

		static void				XMLHandlerStart(void *, const char *, const char **);
		static void				XMLHandlerEnd(void *, const char *);
		Matrix2D				HandleTransformation(BString &value);
		void					HandlePathAttribute(BSVGPath *path, shape_t type, style_s *style);
		void					HandleGradientAttribute(gradient_t type, in_state_s *state, style_s *style);
		void					HandleStopAttribute(BStop *stop, style_s *style);
		void					HandleGroupAttribute(in_state_s *state, style_s *style);

		status_t				InitializeXMLParser();
		status_t				UninitializeXMLParser();

		status_t				ParseXMLData(BPositionIO *data);
		status_t				ParseXMLFile(BFile *file);
		status_t				ParseXMLNode(BNode *node, const char *attr);
		status_t				ParseXMLBuffer(char *buffer, off_t size);

		float					fScale;
		bool					fScaleToFit;
		bool					fFitContent;
		float					fGraphicsWidth;
		float					fGraphicsHeight;
		BRect					fInnerFrame;
		rgb_color				fViewColor;

		BGradient				*fGradient;	// the currently parsed gradient
		BObjectList<in_state_s>	*fInheritStates; // a list of states to inherit

		BBitmap					*fOffscreenBitmap;
		
		bool					fSuperSampling;
		uint32					fSampleSize;

		bool					fOuterFirst;
		BRect					fOuterBounds;

typedef	BView					inherited;
};

inline float
BSVGView::Scale() const
{
	return fScale;
}

inline bool
BSVGView::ScaleToFit() const
{
	return fScaleToFit;
}

inline bool
BSVGView::FitContent() const
{
	return fFitContent;
}

inline bool
BSVGView::SuperSampling() const
{
	return fSuperSampling;
}

inline uint32
BSVGView::SampleSize() const
{
	return fSampleSize;
}

inline float
BSVGView::GraphicsWidth() const
{
	return fGraphicsWidth;
}

inline float
BSVGView::GraphicsHeight() const
{
	return fGraphicsHeight;
}

inline rgb_color
BSVGView::ViewColor()
{
	return fViewColor;
}

inline in_state_s
BSVGView::CurrentState() const
{
	return *fInheritStates->LastItem();
}

#endif	// SVGVIEW_H_
