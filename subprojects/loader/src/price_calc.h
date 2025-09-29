#ifndef SECURITIES_SCANNER_PRICE_CALC_H
#define SECURITIES_SCANNER_PRICE_CALC_H

long calc_price(const long units, const long nano) {
    return units * 100 + nano / 10000000;
}

#endif // SECURITIES_SCANNER_PRICE_CALC_H