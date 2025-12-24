#pragma once


// thank you Sloan Kelly on youtube

namespace Magma
{
	class Server
	{
		public:
			Server();
			~Server();

			void Bind();
			void Update();
			bool GetBound() { return m_Bound; };

		private:
			bool m_Bound;
	};
}