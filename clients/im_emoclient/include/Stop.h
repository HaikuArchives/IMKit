#ifndef STOP_H_
#define STOP_H_

#include <Archivable.h>
#include <GraphicsDefs.h>
#include <Message.h>

class BStop : public BArchivable {

public:
					BStop();
					BStop(BStop *stop);
					BStop(rgb_color color, float offset, float opacity = 1.0);
					BStop(uchar r, uchar g, uchar b, float offset, float opacity = 1.0);
						// offset is a value from 0 to 1; it represents
						// a percentage of the gradient's vector

virtual				~BStop();

					BStop(BMessage *data);
static	BArchivable	*Instantiate(BMessage *data);
virtual	status_t	Archive(BMessage *data, bool deep = true) const;

		void		SetOffset(float offset);
		float		Offset() const;

		void		SetColor(rgb_color color);
		void		SetColor(uchar red, uchar green, uchar blue);
		rgb_color	Color() const;

		void		SetOpacity(float opacity);
		float		Opacity() const;

private:
		float		fOffset;
		rgb_color	fColor;
		float		fOpacity;
};

inline float
BStop::Offset() const
{
	return fOffset;
}

inline rgb_color
BStop::Color() const
{
	rgb_color result = fColor;
	result.alpha = (uchar)(255 * fOpacity);
	return result;
}

inline float
BStop::Opacity() const
{
	return fOpacity;
}

#endif // STOP_H_
