#ifndef SEXYI18N_H
#define SEXYI18N_H

#include <string>

/**
 * @defgroup Internaltionization
 * @{
 */

/**
 * @file
 *
 *  An I18n implementation for the SexyAppFramework
 */

namespace Sexy {

/**
 * mark a translatable string
 *
 * @param x the translatable string
 */
#define tr_noop(x) x

/**
 * mark a translatable string
 *
 * @param c the msgctxt
 * @param x the translatable string
 */
#define tr_cnoop(c, x) x

/**
 * translate a string
 *
 * @param s the string to translate
 *
 * @return a localized string
 */
const char* tr(const char *s);

/**
 * translate a string
 *
 * @param s the string to translate
 *
 * @return a localized string
 */
std::string tr(const std::string &s);

/**
 * translate a string for given message context
 *
 * @param ctx the msgctxt
 * @param s the string to translate
 *
 * @return a localized string
 */
const char* tr(const char *ctx, const char *s);

/**
 * translate a string for given message context
 *
 * @param ctx the msgctxt
 * @param s the string to translate
 *
 * @return a localized string
 */
std::string tr(const std::string &ctx, const std::string &s);

/**
 * translate a string for given domain
 *
 * @param domain the text domain
 * @param s the string to translate
 *
 * @return a localized string
 */
const char* dtr(const char *domain, const char *s);

/**
 * translate a string for given text domain
 *
 * @param domain the text domain
 * @param s the string to translate
 *
 * @return a localized string
 */
std::string dtr(const std::string &domain, const std::string &s);

/**
 * translate a string for given text domain and message context
 *
 * @param domain the domain
 * @param ctx the msgctxt
 * @param s the string to translate
 *
 * @return a localized string
 */
const char* dtr(const char *domain, const char *ctx, const char *s);

/**
 * translate a string for given text domain and message context
 *
 * @param domain the domain
 * @param ctx the msgctxt
 * @param s the string to translate
 *
 * @return a localized string
 */
std::string dtr(const std::string &domain, const std::string &ctx,
		const std::string &s);

/**
 * set the locale of calling process
 *
 * @param locale the locale, can be %NULL
 *
 * @return the current locale of calling process
 */
const char* setLocale(const char *locale);

/**
 * set current text domain
 *
 * @param domain the text domain, can be %NULL
 *
 * @return the current domain
 */
const char* textDomain(const char *domain);

/**
 * sets the base directory of the hierarchy containing message
 * catalogs for a given message domain
 *
 * @param domain the message domain
 * @param dir the base directory to load translated strings
 *
 * @see bindTextDomain(), textDomain()
 */
void bindText(const char *domain, const char *dir);

/**
 * sets the base directory of the hierarchy containing message
 * catalogs for a given message domain
 *
 * @param domain the message domain
 * @param dir the base directory to load translated strings
 *
 *  A message domain is a set of translatable msgid messages. Usually,
 *  every software package has its own message domain. The need for
 *  calling bindTextDomain() arises because packages are not always
 *  installed with the same prefix as the executable. Message catalogs
 *  will be expected at the pathnames dirname/locale/domainname.xml,
 *  where locale is a locale name.
 */
void bindTextDomain(const char *domain, const char *dir);

}

/**
 * @}
 */

#endif
