#ifndef SEXYI18N_H
#define SEXYI18N_H

#include <string>

namespace Sexy {

#define tr_noop(x) x
const char* tr(const char *s);
const std::string tr(const std::string &s);

const char* dtr(const char *domain, const char *s);
const std::string dtr(const std::string &domain, const std::string &s);

const char* setLocale(const char *locale);
const char* textDomain(const char *domain);
void bindText(const char *domain, const char *dir);
void bindTextDomain(const char *domain, const char *dir);

}

#endif
