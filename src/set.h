//simple set operation

#ifndef SET_H
#define SET_H

template <int maxElem> class Set
{
	public:
		// { 0 .. maxElem }
		// Construct { }
		Set()
		{
			clear();
		}
		// Construct { e1 }
		Set(int e1)
		{
			clear();
			incl(e1);
		}
		// Construct { e1, e2 }
		Set(int e1, int e2)
		{
			clear();
			incl(e1); incl(e2);
		}
		// Construct { e1, e2, e3 }
		Set(int e1, int e2, int e3)
		{
			clear();
			incl(e1);
			incl(e2);
			incl(e3);
		}
		// Construct { e[0] .. e[n-1] }
		Set(int n, int e1[])
		{
			clear();
			for (int i = 0; i < n; i++)
				incl(e1[i]);
		}
		// Include e
		void incl(int e)
		{
			if(e >= 0 && e <= maxElem)
				bits[wrd(e)] |= bitmask(e);
		}
		// Exclude e
		void excl(int e)
		{
			if(e >= 0 && e <= maxElem)
				bits[wrd(e)] &= ~bitmask(e);
		}
		// Test membership for e
		int memb(int e)
		{
			if(e >= 0 && e <= maxElem)
				return((bits[wrd(e)] & bitmask(e)) != 0);
			else
				return 0;
		}
		// Test for empty set
		int isempty(void)
		{
			for (int i = 0; i < length; i++)
				if (bits[i])
					return 0;
			return 1;
		}
		// Union with s
		Set operator + (const Set &s)
		{
			Set<maxElem> r;
			for(int i = 0; i < length; i++)
				r.bits[i] = bits[i] | s.bits[i];
			return r;
		}
		// Intersection with s
		Set operator * (const Set &s)
		{
			Set<maxElem> r;
			for(int i = 0; i < length; i++)
				r.bits[i] = bits[i] & s.bits[i];
			return r;
		}
		// Difference with s
		Set operator - (const Set &s)
		{
			Set<maxElem> r;
			for(int i = 0; i < length; i++)
				r.bits[i] = bits[i] & ~s.bits[i];
			return r;
		}
		// Symmetric difference with s
		Set operator / (const Set &s)
		{
			Set<maxElem> r;
			for(int i = 0; i < length; i++)
				r.bits[i] = bits[i] ^ s.bits[i];
			return r;
		}
	private:
		unsigned char bits[(maxElem + 8) / 8];
		int length;
		int wrd(int i)
		{
			return(i / 8);
		}
		int bitmask(int i)
		{
			return(1 << (i % 8));
		}
		void clear()
		{
			length = (maxElem + 8) / 8;
			for(int i = 0; i < length; i++)
				bits[i] = 0;
		}
};

#endif /* SET_H */