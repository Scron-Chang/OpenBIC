#include <stdio.h>
#include <stdlib.h>

#include <fs/fs.h>
#include <fs/littlefs.h>

#define PARTITION_NODE DT_NODELABEL(lfs1)
FS_FSTAB_DECLARE_ENTRY(PARTITION_NODE);

#define FILENAME "sel"

void plat_sel_init(void)
{
    struct fs_mount_t *mp = &FS_FSTAB_ENTRY(PARTITION_NODE);
    struct fs_dirent dirent;
    struct fs_file_t fp;
    char fname[256] = { 0 };
    int rc = 0;

	unsigned int id = (uintptr_t)mp->storage_dev;
    const struct flash_area *pfa;
    rc = flash_area_open(id, &pfa);

	if (IS_ENABLED(CONFIG_APP_WIPE_STORAGE)) {
		printk("Erasing flash area ... ");
		rc = flash_area_erase(pfa, 0, pfa->fa_size);
		printk("%d\n", rc);
	}

#ifdef CHECK_FLASH_AREA
	if (rc < 0) {
		printf("FAIL: unable to find flash area %u: %d\n",
		       id, rc);
		return;
	}
    //Area 0 at 0x40000 on fmc_cs0 for 786432 bytes
    printk("Area %u at 0x%x on %s for %u bytes\n",
	       id, (unsigned int)pfa->fa_off, pfa->fa_dev_name,
	       (unsigned int)pfa->fa_size);

    flash_area_close(pfa);
#endif

#ifdef CHECK_FS_STATUS
    struct fs_statvfs sbuf;
    rc = fs_statvfs(mp->mnt_point, &sbuf);
	if (rc < 0) {
		printk("FAIL: statvfs: %d\n", rc);
		return;
	}
    // /lfs1: bsize = 16 ; frsize = 65536 ; blocks = 12 ; bfree = 7
	printk("%s: bsize = %lu ; frsize = %lu ; blocks = %lu ; bfree = %lu\n",
	       mp->mnt_point,
	       sbuf.f_bsize, sbuf.f_frsize, sbuf.f_blocks, sbuf.f_bfree);
#endif



    fs_file_t_init(&fp);
    snprintf(fname, sizeof(fname), "%s/"FILENAME, mp->mnt_point);

	if ((rc = fs_stat(fname, &dirent)) >= 0) {
		printk("%s size is %u\n", dirent.name, dirent.size);
	}
    else if (rc == LFS_ERR_NOENT)
    {
        printf("File %s not found. Need to create a new one.\n", fname);
    }
}

__attribute__((unused)) void plat_sel_sample(void)
{
    struct fs_mount_t *mp = &FS_FSTAB_ENTRY(PARTITION_NODE);
    unsigned int id = (uintptr_t)mp->storage_dev;
    char fname[256] = { 0 };
    const struct flash_area *pfa;
    struct fs_statvfs sbuf;

    snprintf(fname, sizeof(fname), "%s/boot_count", mp->mnt_point);

	int rc = flash_area_open(id, &pfa);
	if (rc < 0) {
		printf("FAIL: unable to find flash area %u: %d\n",
		       id, rc);
		return;
	}

	printk("Area %u at 0x%x on %s for %u bytes\n",
	       id, (unsigned int)pfa->fa_off, pfa->fa_dev_name,
	       (unsigned int)pfa->fa_size);

    flash_area_close(pfa);

    rc = fs_statvfs(mp->mnt_point, &sbuf);
	if (rc < 0) {
		printk("FAIL: statvfs: %d\n", rc);
		goto out;
	}

	printk("%s: bsize = %lu ; frsize = %lu ;"
	       " blocks = %lu ; bfree = %lu\n",
	       mp->mnt_point,
	       sbuf.f_bsize, sbuf.f_frsize,
	       sbuf.f_blocks, sbuf.f_bfree);

	struct fs_dirent dirent;
    // /lfs1/boot_count stat: 0
    //         fn 'boot_count' siz 4
	rc = fs_stat(fname, &dirent);
	printk("%s stat: %d\n", fname, rc);
	if (rc >= 0) {
		printk("\tfn '%s' siz %u\n", dirent.name, dirent.size);
	}

	struct fs_file_t file;

	fs_file_t_init(&file);

	rc = fs_open(&file, fname, FS_O_CREATE | FS_O_RDWR);
	if (rc < 0) {
		printk("FAIL: open %s: %d\n", fname, rc);
		goto out;
	}
    uint32_t boot_count = 0;

	if (rc >= 0) {
		rc = fs_read(&file, &boot_count, sizeof(boot_count));
		printk("%s read count %u: %d\n", fname, boot_count, rc);
		rc = fs_seek(&file, 0, FS_SEEK_SET);
		printk("%s seek start: %d\n", fname, rc);

	}

	boot_count += 1;
	rc = fs_write(&file, &boot_count, sizeof(boot_count));
	printk("%s write new boot count %u: %d\n", fname,
	       boot_count, rc);

	rc = fs_close(&file);
	printk("%s close: %d\n", fname, rc);

	struct fs_dir_t dir;

	fs_dir_t_init(&dir);

	rc = fs_opendir(&dir, mp->mnt_point);
	printk("%s opendir: %d\n", mp->mnt_point, rc);

	while (rc >= 0) {
		struct fs_dirent ent = { 0 };

		rc = fs_readdir(&dir, &ent);
		if (rc < 0) {
			break;
		}
		if (ent.name[0] == 0) {
			printk("End of files\n");
			break;
		}
		printk("  %c %u %s\n",
		       (ent.type == FS_DIR_ENTRY_FILE) ? 'F' : 'D',
		       ent.size,
		       ent.name);
	}

	(void)fs_closedir(&dir);

out:
	rc = fs_unmount(mp);
	printk("%s unmount: %d\n", mp->mnt_point, rc);
}
