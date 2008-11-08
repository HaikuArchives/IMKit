#ifndef STATUSICON_H
#define STATUSICON_H

#include <support/DataIO.h>

namespace IM {
	class StatusIcon {
		public:
								StatusIcon(void);
								~StatusIcon(void);
									
			void				SetVectorIcon(const void *data, ssize_t size);
			const void			*VectorIcon(void);
			ssize_t				VectorIconSize(void);
			
			void				SetMiniIcon(const void *data, ssize_t size);
			const void			* MiniIcon(void);
			ssize_t				MiniIconSize(void);
	
			void				SetLargeIcon(const void *data, ssize_t size);
			const void			*LargeIcon(void);
			ssize_t				LargeIconSize(void);
	
			bool				IsEmpty(void);
	
		private:
			void				*fVectorIcon;
			ssize_t				fVectorIconSize;
			void 				*fMiniIcon;
			ssize_t				fMiniIconSize;
			void				*fLargeIcon;
			ssize_t				fLargeIconSize;
	};
};
		
#endif