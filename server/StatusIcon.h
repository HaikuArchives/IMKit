#ifndef STATUSICON_H
#define STATUSICON_H

#include <support/DataIO.h>

namespace IM {
	class StatusIcon {
		public:
								StatusIcon(void);
								~StatusIcon(void);
									
			void				SetVectorIcon(const void *data, size_t size);
			const void			*VectorIcon(void);
			size_t				VectorIconSize(void);
			
			void				SetMiniIcon(const void *data, size_t size);
			const void			* MiniIcon(void);
			size_t				MiniIconSize(void);
	
			void				SetLargeIcon(const void *data, size_t size);
			const void			*LargeIcon(void);
			size_t				LargeIconSize(void);
	
			bool				IsEmpty(void);
	
		private:
			void				*fVectorIcon;
			size_t				fVectorIconSize;
			void 				*fMiniIcon;
			size_t				fMiniIconSize;
			void				*fLargeIcon;
			size_t				fLargeIconSize;
	};
};
		
#endif