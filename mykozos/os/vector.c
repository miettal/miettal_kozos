#include "defines.h"

extern void start(void);        /* スタート・アップ */
extern void intr_softerr(void); /* ソフトウエア・エラー */
extern void intr_syscall(void); /* システム・コール */
extern void intr_serintr(void); /* シリアル割込み */
extern void intr_timintr(void); /* タイマ割込み */
extern void intr_ethintr(void); /* イーサネット・コントローラ割込み */
extern void intr_bkpoint(void); /* デバッガ用ブレークポイント */

/*
 * 割込みベクタの設定．
 * リンカ・スクリプトの定義により，先頭番地に配置される．
 */
void (*vectors[])(void) = {
  start, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
  intr_syscall, intr_softerr, intr_softerr, intr_bkpoint,
  NULL, NULL, NULL, NULL, NULL, intr_ethintr /* IRQ5 */, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  intr_timintr, intr_timintr, intr_timintr, intr_timintr,
  intr_timintr, intr_timintr, intr_timintr, intr_timintr,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  intr_serintr, intr_serintr, intr_serintr, intr_serintr,
  intr_serintr, intr_serintr, intr_serintr, intr_serintr,
  intr_serintr, intr_serintr, intr_serintr, intr_serintr,
};
