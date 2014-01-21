#include <stdlib.h>
#include <string.h>

#define USE_RINTERNALS
#include <Rinternals.h>

/* from point_poly.cpp */
void *load_shape_(double *x, double *y, int n, int *parts, int parts_n);
int   inside_shape_(void *s, double x, double y);
void  free_shape_(void *s);

SEXP shp_inside(SEXP slist, SEXP pxv, SEXP pyv, SEXP clockw) {
    SEXP xv, yv, pv, res;
    double *x, *y, *px, *py;
    int *p, up = 0, *r, np, ns, mp = 0, i,
      expected = (asInteger(clockw) == TRUE) ? 1 : -1;
    if (TYPEOF(slist) != VECSXP || !inherits(slist, "shp"))
	Rf_error("input must be a list of shapes (shp object)");
    if (LENGTH(slist) == 0)
	return allocVector(INTSXP, 0);
    if (LENGTH(pxv) != LENGTH(pyv))
	Rf_error("point coordinates must have the same length");
    if (TYPEOF(pxv) != REALSXP) {
	pxv = PROTECT(coerceVector(pxv, REALSXP)); up++;
    }
    if (TYPEOF(pyv) != REALSXP) {
	pyv = PROTECT(coerceVector(pyv, REALSXP)); up++;
    }
    px = REAL(pxv);
    py = REAL(pyv);
    np = LENGTH(pxv);
    res = allocVector(INTSXP, np);
    r = INTEGER(res);
    memset(r, 0, sizeof(*r) * np);
    ns = LENGTH(slist);
    for (i = 0; i < ns; i++) {
	int j;
	double *bb;
	void *shape = 0;
	SEXP shp = VECTOR_ELT(slist, i);
	bb = REAL(VECTOR_ELT(shp, 2));
	pv = VECTOR_ELT(shp, 3); p = INTEGER(pv);
	xv = VECTOR_ELT(shp, 4); x = REAL(xv);
	yv = VECTOR_ELT(shp, 5); y = REAL(yv);
	for (j = 0; j < np; j++) {
	    double X = px[j], Y = py[j];
	    /* is the point inside the bounding box? */
	    if (X >= bb[0] && X <= bb[2] && Y >= bb[1] && Y <= bb[3]) {
		/* lazy-load shapes */
		if (!shape)
		    shape = load_shape_(x, y, LENGTH(xv), p, LENGTH(pv));

		/* then use point/poly test */
		if (inside_shape_(shape, X, Y) && !r[j]) {
		    mp++;
		    r[j] = i + 1;
		    if (mp >= np) { /* if all points got matched, get out */
			i = ns;
			break;
		    }
		}
	    }
	}
	if (shape)
	    free_shape_(shape);
    }
    if (mp < np) /* replace 0 (no match) with NA */
	for (i = 0; i < np; i++)
	    if (r[i] == 0) r[i] = NA_INTEGER;
    if (up) UNPROTECT(up);
    return res;
}
