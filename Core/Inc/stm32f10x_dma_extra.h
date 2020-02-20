#pragma once

#define  DMA_CCRx_EN                         ((uint16_t)0x0001)            /*!< Channel enable */
#define  DMA_CCRx_TCIE                       ((uint16_t)0x0002)            /*!< Transfer complete interrupt enable */
#define  DMA_CCRx_HTIE                       ((uint16_t)0x0004)            /*!< Half Transfer interrupt enable */
#define  DMA_CCRx_TEIE                       ((uint16_t)0x0008)            /*!< Transfer error interrupt enable */
#define  DMA_CCRx_DIR                        ((uint16_t)0x0010)            /*!< Data transfer direction */
#define  DMA_CCRx_CIRC                       ((uint16_t)0x0020)            /*!< Circular mode */
#define  DMA_CCRx_PINC                       ((uint16_t)0x0040)            /*!< Peripheral increment mode */
#define  DMA_CCRx_MINC                       ((uint16_t)0x0080)            /*!< Memory increment mode */

#define  DMA_CCRx_PSIZE                      ((uint16_t)0x0300)            /*!< PSIZE[1:0] bits (Peripheral size) */
#define  DMA_CCRx_PSIZE_0                    ((uint16_t)0x0100)            /*!< Bit 0 */
#define  DMA_CCRx_PSIZE_1                    ((uint16_t)0x0200)            /*!< Bit 1 */

#define  DMA_CCRx_MSIZE                      ((uint16_t)0x0C00)            /*!< MSIZE[1:0] bits (Memory size) */
#define  DMA_CCRx_MSIZE_0                    ((uint16_t)0x0400)            /*!< Bit 0 */
#define  DMA_CCRx_MSIZE_1                    ((uint16_t)0x0800)            /*!< Bit 1 */

#define  DMA_CCRx_PL                         ((uint16_t)0x3000)            /*!< PL[1:0] bits (Channel Priority level) */
#define  DMA_CCRx_PL_0                       ((uint16_t)0x1000)            /*!< Bit 0 */
#define  DMA_CCRx_PL_1                       ((uint16_t)0x2000)            /*!< Bit 1 */
