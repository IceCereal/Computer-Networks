#ifndef COMMON_H_
#define COMMON_H_

/*
	IndexGet ShortList: 1
	IndexGet LongList: 2
	IndexGet RegEx: 3
	FileHash Verify: 4
	FileUpload: 5
	FileDownload: 6
*/

struct InitData{
	int type;
};

// IndexGet ShortList TimeStart TimeEnd
struct ShortList_Data{
	long int startModTime; // Start of Last Modified Time
	long int endModTime; // End of Last Modified Time
};

// IndexGet RegEx "RegEx"
struct RegEx_Data{
	char regex[256];
};

// FileHash Verify FileName
struct FileHashVerify_Data{
	char filename[256];
};

// FileUpload Filename
struct FileUpload_Data{
	char filename[256];
	long int filesize;
};

// FileDownload Filename
struct FileDownload_Data{
	char filename[256];
};

/* Server Sided Structures */
// FileDownload Filename
struct FileDownload_Return_Data{
	unsigned int status;
	char filename[256];
	long int filesize;
};


#endif