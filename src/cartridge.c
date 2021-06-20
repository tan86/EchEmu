#include "cartridge.h"

enum mirror mirroringcheck(cartridge *cart) {
  if (cart->control1 & (1 << 3))
    return FOURSCRMIR;
  else if (cart->control1 & (1 << 0))
    return VERTICALMIR;
  else
    return HORIZONTALMIR;
}

bool batteryramcheck(cartridge *cart) { return (cart->control1 & (1 << 1)); }

bool trainercheck(cartridge *cart) { return (cart->control1 & (1 << 2)); }

int mapperinfo(cartridge *cart) {
  return ((cart->control1 & 0xf0) >> 4) | ((cart->control2 & 0xf0) >> 0);
}

void loadcart(cartridge *cart, char *file) {
  FILE *game = fopen(file, "rb");
  if (!game) {
    printf("Can't open the file %s \n", file);
    exit(1);
  }
  fread(cart, INESHEADERSIZE, 1, game);
  char tempsign[5] = {'N', 'E', 'S', 0x1a, '\0'};
  if (!(strcmp(cart->type, tempsign))) {
    printf("Invalied File!!\n");
    exit(1);
  }
  if (1) {
    if (!(cart->batteryrampointer = (char *)malloc(BATTERYRAMSIZE))) {
      printf("Failed allocation for battery ram\n");
      exit(1);
    }
  }
  if (trainercheck(cart)) {
    cart->trainerpointer = (char *)malloc(TRAINERSIZE);
    if (!cart->trainerpointer) {
      printf("Failed allocation memory for trainer\n");
      exit(1);
    } else
      fread(cart->trainerpointer, TRAINERSIZE, 1, game);
  }
  if (cart->prgromno > 0) {
    cart->prgrompointer = (char *)malloc(cart->prgromno * PRGROMSIZE);
    if (!cart->prgrompointer) {
      printf("Failed allocation for PRG-ROM\n");
      exit(1);
    } else
      fread(cart->prgrompointer, cart->prgromno * 0x4000, 1, game);
  }

  if (cart->chrromno)
    cart->chrrampresent = 0;
  else {
    cart->chrrampresent = 1;
    cart->chrromno      = 1;
  }
  cart->chrrompointer = (char *)malloc(cart->chrromno * CHRROMSIZE);
  if (!cart->chrrompointer) {
    printf("Failed allocation for CHRROM\n");
    exit(1);
  }

  if (cart->chrrampresent)
    memset(cart->chrrompointer, 0, cart->chrromno * 0x2000);
  else
    fread(cart->chrrompointer, cart->chrromno * CHRROMSIZE, 1, game);

  cart->mir = mirroringcheck(cart);

  fclose(game);
}