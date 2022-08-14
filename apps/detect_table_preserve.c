/* Detecs if EFI system table, runtimes services, and boot services pointers get preserved through StartImage/LoadImage */

#include <efi.h>
#include <efilib.h>

EFI_FILE_HANDLE GetVolume(EFI_HANDLE image)
{
  EFI_LOADED_IMAGE *loaded_image = NULL;                  /* image interface */
  EFI_GUID lipGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;      /* image interface GUID */
  EFI_FILE_IO_INTERFACE *IOVolume;                        /* file system interface */
  EFI_GUID fsGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID; /* file system interface GUID */
  EFI_FILE_HANDLE Volume;                                 /* the volume's interface */

  /* get the loaded image protocol interface for our "image" */
  uefi_call_wrapper(BS->HandleProtocol, 3, image, &lipGuid, (void **) &loaded_image);
  /* get the volume handle */
  uefi_call_wrapper(BS->HandleProtocol, 3, loaded_image->DeviceHandle, &fsGuid, (VOID*)&IOVolume);
  uefi_call_wrapper(IOVolume->OpenVolume, 2, IOVolume, &Volume);
  return Volume;
}

UINT64 FileSize(EFI_FILE_HANDLE FileHandle)
{
  UINT64 ret;
  EFI_FILE_INFO       *FileInfo;         /* file information structure */
  /* get the file's size */
  FileInfo = LibFileInfo(FileHandle);
  ret = FileInfo->FileSize;
  FreePool(FileInfo);
  return ret;
}

EFI_STATUS
efi_main (EFI_HANDLE image_handle, EFI_SYSTEM_TABLE *systab)
{
	EFI_STATUS status;

	InitializeLib(image_handle, systab);
	Print((CHAR16 *)L"Current EFI system table pointer: %x\r\n", systab);
	Print((CHAR16 *)L"Current EFI runtime services pointer: %x\r\n", systab->RuntimeServices);
	Print((CHAR16 *)L"Current EFI boot services pointer: %x\r\n", systab->BootServices);

	/* Load binary into memory */
	EFI_FILE_HANDLE current_volume = GetVolume(image_handle);
	CHAR16 *file_name = L"EFI\\preserve_tables.efi";
	EFI_FILE_HANDLE image_file;
	uefi_call_wrapper(current_volume->Open, 5, current_volume, &image_file, file_name, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM);
	UINT64 image_file_size = FileSize(image_file);
	UINT8 *image_file_buffer = AllocatePool(image_file_size);
	uefi_call_wrapper(image_file->Read, 3, image_file, &image_file_size, image_file_buffer);
	Print((CHAR16 *)L"Read %d bytes\r\n", image_file_size);
	uefi_call_wrapper(image_file->Close, 1, image_file);

	/* Execute binary */
	EFI_HANDLE image_file_image;
	MEMMAP_DEVICE_PATH mempath[2];
	mempath[0].Header.Type = HARDWARE_DEVICE_PATH;
	mempath[0].Header.SubType = HW_MEMMAP_DP;
	mempath[0].Header.Length[0] = (UINT8)sizeof(MEMMAP_DEVICE_PATH);
	mempath[0].Header.Length[1] = (UINT8)(sizeof(MEMMAP_DEVICE_PATH)>> 8);
	mempath[0].MemoryType = EfiLoaderCode;
	mempath[0].StartingAddress = (EFI_PHYSICAL_ADDRESS) image_file_buffer;
	mempath[0].EndingAddress = (EFI_PHYSICAL_ADDRESS) (image_file_buffer + image_file_size);
	mempath[1].Header.Type = END_DEVICE_PATH_TYPE;
	mempath[1].Header.SubType = END_INSTANCE_DEVICE_PATH_SUBTYPE;
	mempath[1].Header.Length[0] = (UINT8)sizeof(EFI_DEVICE_PATH);
	mempath[1].Header.Length[1] = (UINT8)(sizeof(EFI_DEVICE_PATH)>> 8);
	status = uefi_call_wrapper(systab->BootServices->LoadImage, FALSE, FALSE, image_handle, (EFI_DEVICE_PATH *)mempath, image_file_buffer, image_file_size, &image_file_image);
	if (EFI_ERROR(status)) {
		Print((CHAR16*)L"Could not LoadImage %r %x \r\n", status, status);
	}

	FreePool(image_file_buffer);
	return EFI_SUCCESS;
}
 
