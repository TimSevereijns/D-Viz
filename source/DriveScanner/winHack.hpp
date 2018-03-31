#ifndef WINHACK_HPP
#define WINHACK_HPP

#ifdef Q_OS_WIN
   #include <minwindef.h>

   // @note Due to some abysmal design choices on Microsoft's part, we can't include WDM, WinNT, or
   // NTIFS in the same project, since they all managed to define the same macros. Stripping only
   // the necessary definitions from those files lets me get around this silliness.
   //
   // Inspired by: https://github.com/google/symboliclink-testing-tools

   typedef struct _REPARSE_DATA_BUFFER {
      ULONG  ReparseTag;
      USHORT ReparseDataLength;
      USHORT Reserved;
      union {
         struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            ULONG Flags;
            WCHAR PathBuffer[1];
         } SymbolicLinkReparseBuffer;
         struct {
            USHORT SubstituteNameOffset;
            USHORT SubstituteNameLength;
            USHORT PrintNameOffset;
            USHORT PrintNameLength;
            WCHAR PathBuffer[1];
         } MountPointReparseBuffer;
         struct {
            UCHAR  DataBuffer[1];
         } GenericReparseBuffer;
      } DUMMYUNIONNAME;
   } REPARSE_DATA_BUFFER, *PREPARSE_DATA_BUFFER;

   #define REPARSE_DATA_BUFFER_HEADER_LENGTH FIELD_OFFSET(REPARSE_DATA_BUFFER, \
      GenericReparseBuffer.DataBuffer)

   // @see https://msdn.microsoft.com/en-us/library/windows/desktop/aa365511(v=vs.85).aspx

   #define IO_REPARSE_TAG_MOUNT_POINT              (0xA0000003L)       // winnt
   #define IO_REPARSE_TAG_HSM                      (0xC0000004L)       // winnt
   #define IO_REPARSE_TAG_DRIVE_EXTENDER           (0x80000005L)
   #define IO_REPARSE_TAG_HSM2                     (0x80000006L)       // winnt
   #define IO_REPARSE_TAG_SIS                      (0x80000007L)       // winnt
   #define IO_REPARSE_TAG_WIM                      (0x80000008L)       // winnt
   #define IO_REPARSE_TAG_CSV                      (0x80000009L)       // winnt
   #define IO_REPARSE_TAG_DFS                      (0x8000000AL)       // winnt
   #define IO_REPARSE_TAG_FILTER_MANAGER           (0x8000000BL)
   #define IO_REPARSE_TAG_SYMLINK                  (0xA000000CL)       // winnt
   #define IO_REPARSE_TAG_IIS_CACHE                (0xA0000010L)
   #define IO_REPARSE_TAG_DFSR                     (0x80000012L)       // winnt
   #define IO_REPARSE_TAG_DEDUP                    (0x80000013L)       // winnt
   #define IO_REPARSE_TAG_APPXSTRM                 (0xC0000014L)
   #define IO_REPARSE_TAG_NFS                      (0x80000014L)       // winnt
   #define IO_REPARSE_TAG_FILE_PLACEHOLDER         (0x80000015L)       // winnt
   #define IO_REPARSE_TAG_DFM                      (0x80000016L)
   #define IO_REPARSE_TAG_WOF                      (0x80000017L)       // winnt
#endif

#endif // WINHACK_HPP
