#ifndef _TVECTOR_H
#define _TVECTOR_H

class Track;

struct Vector {
	int	_size;
	int	_maxelem;
	TrackBase **_ptr;
	char	*_flags;		/* direction of this track */
	long	_pathlen;		/* length in meters of this path */
	long    _distostp;

        Vector  *_nextFree;

        Vector(int hint = 0);
        ~Vector();

	Track	*TrackAt(int index);	// get object at index
        Track   *FirstTrack();
        Track   *LastTrack();
	int	FlagAt(int index);
	void	Empty();
	void	Add(TrackBase *trk, int flag);
	void	DeleteAt(int index);
	void	DeleteTrack(TrackBase *trk);
	int	Find(TrackBase *trk);
	void	Reverse();
	void	Insert(TrackBase *trk, int f);// insert new element at the beginning
	void	Insert(Vector *newPath);// insert new path at the beginning
	void	ComputeLength();	// length in meters of path
	void    ContainBusy(int len);      // distance to bust track    by ypx
};

#endif // _TVECTOR_H
