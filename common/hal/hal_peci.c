#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <drivers/peci.h>
#include "hal_peci.h"
#include "libutil.h"

const struct device *dev;

struct k_mutex peci_lock;
struct k_timer retry_timer;

int peci_init()
{
	dev = device_get_binding("PECI");
	int ret;
	uint32_t bitrate = 1000;
	if (!dev) {
		printf("peci device not found");
		return 0;
	}
	ret = peci_config(dev, bitrate);
	if (ret) {
		printf("set bitrate %dKbps failed %d\n", bitrate, ret);
		return ret;
	}
	ret = peci_enable(dev);
	if (ret) {
		printf("peci enable failed %d\n", ret);
		return ret;
	}
	k_mutex_init(&peci_lock);
	k_timer_init(&retry_timer, NULL, NULL);

	return ret;
}

int peci_ping(uint8_t address)
{
	struct peci_msg pkgcfg;
	int ret;

	memset(&pkgcfg, 0, sizeof(struct peci_msg));
	pkgcfg.addr = address;
	pkgcfg.cmd_code = PECI_CMD_PING;

	ret = peci_transfer(dev, &pkgcfg);
	if (ret) {
		printf("[%s] Failed to send the PECI PING command(0x%x), status: %d\n", __func__,
		       pkgcfg.cmd_code, ret);
	}

	return ret;
}

/*
* For some commands, the PECI originator may need to retry a command if
* the processor PECI client responds with a 0x8x completion code. In
* each instance, the processor PECI client may have started the
* operation but not completed it yet. When the 'retry' bit is set, the
* PECI client will ignore a new request if it exactly matches a
* previous valid request. For better performance and for reducing
* retry traffic, the interval time will be increased exponentially.
*/
int peci_xfer_with_retries(struct peci_msg *msg)
{
	int interval = PECI_DEV_RETRY_INTERVAL_MIN_MSEC;
	int ret = 0;

	if (msg == NULL) {
		printf("%s(): NULL PECI command\n", __func__);
		return -1;
	}

	k_timer_start(&retry_timer, K_MSEC(PECI_DEV_RETRY_TIMEOUT), K_NO_WAIT);
	while (1) {
		ret = peci_transfer(dev, msg);
		if ((ret != 0) || (msg->rx_buffer.buf == NULL)) {
			break;
		}
		/* Retry is needed when completion code is 0x8x */
		if (!IS_PECI_CC_NEED_RETRY(msg->rx_buffer.buf[0])) {
			break;
		}
		/* Set the retry bit to indicate a retry attempt */
		msg->tx_buffer.buf[0] |= PECI_DEV_RETRY_BIT;

		/* Retry it for 'timeout' before returning an error. */
		if (k_timer_status_get(&retry_timer) > 0) {
			printf("%s(): Timeout retrying transfer!\n", __func__);
			break;
		}

		k_msleep(interval);

		interval *= 2;
		if (interval > PECI_DEV_RETRY_INTERVAL_MAX_MSEC) {
			interval = PECI_DEV_RETRY_INTERVAL_MAX_MSEC;
		}
	}
	return ret;
}

int peci_cmd_xfer(struct peci_msg *msg)
{
	int ret = 0;

	if (msg == NULL) {
		printf("%s(): NULL PECI command\n", __func__);
		return -1;
	}
	if (k_mutex_lock(&peci_lock, K_MSEC(1000)) != 0) {
		printf("%s(): Can not get lock\n", __func__);
		return -1;
	}
	switch (msg->cmd_code) {
	case PECI_CMD_GET_DIB:
	case PECI_CMD_GET_TEMP0:
	case PECI_CMD_GET_TEMP1:
		ret = peci_transfer(dev, msg);
		break;
	default:
		ret = peci_xfer_with_retries(msg);
	}
	k_mutex_unlock(&peci_lock);
	return ret;
}

int peci_read(uint8_t cmd, uint8_t address, uint8_t u8Index, uint16_t u16Param, uint8_t u8ReadLen,
	      uint8_t *readBuf)
{
	struct peci_msg rdpkgcfg;
	int ret;

	if (readBuf == NULL) {
		printf("PECI read buffer was passed in as null\n");
		return -1;
	}
	rdpkgcfg.cmd_code = cmd;
	rdpkgcfg.addr = address;
	rdpkgcfg.tx_buffer.len = 0x05;
	rdpkgcfg.rx_buffer.len = u8ReadLen;
	rdpkgcfg.tx_buffer.buf = (uint8_t *)malloc(rdpkgcfg.tx_buffer.len * sizeof(uint8_t));
	if (rdpkgcfg.tx_buffer.buf == NULL) {
		printf("Could not initialize memory for tx_buffer\n");
		return -1;
	}
	rdpkgcfg.rx_buffer.buf = readBuf;
	rdpkgcfg.tx_buffer.buf[0] = 0x00;
	rdpkgcfg.tx_buffer.buf[1] = u8Index;
	rdpkgcfg.tx_buffer.buf[2] = u16Param & 0xff;
	rdpkgcfg.tx_buffer.buf[3] = u16Param >> 8;
	ret = peci_cmd_xfer(&rdpkgcfg);

	if (DEBUG_PECI) {
		uint8_t index;
		for (index = 0; index < 5; index++)
			printf("%02x ", readBuf[index]);
		printf("\n");
	}

	if (ret) {
		printf("[%s] Failed to send PECI Command(0x%x), status: %d\n", __func__,
		       rdpkgcfg.cmd_code, ret);
	}

	SAFE_FREE(rdpkgcfg.tx_buffer.buf);
	return ret;
}

int peci_write(uint8_t cmd, uint8_t address, uint8_t u8ReadLen, uint8_t *readBuf,
	       uint8_t u8WriteLen, uint8_t *writeBuf)
{
	if ((readBuf == NULL) || (writeBuf == NULL)) {
		return -1;
	}

	struct peci_msg wrpkgcfg;
	int ret;

	wrpkgcfg.addr = address;
	wrpkgcfg.cmd_code = cmd;
	wrpkgcfg.tx_buffer.len = u8WriteLen;
	wrpkgcfg.rx_buffer.len = u8ReadLen;
	wrpkgcfg.rx_buffer.buf = readBuf;
	wrpkgcfg.tx_buffer.buf = writeBuf;

	ret = peci_cmd_xfer(&wrpkgcfg);
	if (ret) {
		printf("peci write failed %d\n", ret);
		return ret;
	}

	return ret;
}
