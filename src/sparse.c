/*********************************************************************************
  *Copyright(C)     hxGPT && USTB
  *FileName:        sparse.c
  *Author:          Tailin (Terrance) Liang 
  *Contact:         tailin.liang@outlook.com
  *Version:         1.0
  *Date:            2018.11.12
  *Description:     transfer sparse matrix to csr format
  *Others:
  *Function List:
  *History:
     1.Date:
       Author:
       Modification:
     2.…………
**********************************************************************************/
/* users */
#include "sparse.h"

/* for fun */
#define vodi void
#define viod void
#define retrun return
#define mian main

/* for sparse mat */
#define index(a) ((a).col_index)       //get index in csr
#define offset(a) ((a).offset)         //get offset in csr
#define offset_row(a) ((a).offset_row) //self defined offset 190219
#define smval(a) (a->val)              //get vals in csr, using like smval(a)[i]

/* for ele */
#define val(a) ((a).val)
#define col(a) ((a).p.col)
#define row(a) ((a).p.row)
//----------------------------------------COMPRESSION----------------------------------------//

/*  Function name:  vec2ele
    Discreption:    transfer none-zero data in sparse vectors into elements structs.
    Parameters:     
        @vec        input vector.
        @num_row    row number of matrix like vector.
        @num_col    row number of matrix like vector.
    Return:         A group of elements struct with non-zero data coordinates and values.
*/
ele *vec2ele(val_t *vec, count_t num_row, count_t num_col)
{
    ele *e = calloc(num_row * num_col, sizeof(ele));
    count_t k = 0;
    for (count_t i = 0; i < num_row; i++)
    {
        for (count_t j = 0; j < num_col; j++)
        {
            if (vec[i * num_col + j])
            {
                col(e[k]) = j;
                row(e[k]) = i;
                val(e[k]) = vec[i * num_col + j];
                k += 1;
            }
        }
    }
    return e;
}

/*  Function name:  ele2csr
    Discreption:    transfer none-zero elements to csr style.
    Parameters:
        @e          input elements without zero.
        @num_row    row number of matrix like vector.
        @num_col    row number of matrix like vector.
    Return:         CSR style storage.
*/
spa_mat ele2csr(ele *e, count_t num_row, count_t num_col, count_t num_ele)
{
    count_t ofst = 0;
    count_t last_nzd = 0;
    spa_mat spmt = {};

    /*spmt init*/
    spmt.num_nzd = 0,
    spmt.num_zd = 0;
    spmt.num_row = num_row;
    spmt.num_col = num_col;
    spmt.val = calloc(num_ele, sizeof(val_t));
    spmt.col_index = calloc(num_ele, sizeof(index_t));
    spmt.offset = calloc(num_row + 1, sizeof(offset_t));
    spmt.offset_row = calloc(num_row, sizeof(offset_row_t));

    /* put into CSR */
    for (count_t i = 0; i < num_ele; i++)
    // for (count_t i = 0; i < num_row * num_col; i++)
    {
        if (val(e[i]) != 0) //actually every element != 0
        {
            if (row(e[i]) > row(e[last_nzd]) || !spmt.num_nzd && !last_nzd)
            {
                offset(spmt)[ofst] = spmt.num_nzd;
                if (ofst > 0)
                    offset_row(spmt)[ofst - 1] = offset(spmt)[ofst] - offset(spmt)[ofst - 1];
                ofst += 1;
            }
            val(spmt)[spmt.num_nzd] = val(e[i]);
            index(spmt)[spmt.num_nzd] = col(e[i]);
            spmt.num_nzd += 1;
            last_nzd = i;
        }
        else
            spmt.num_zd += 1;
    }
    //finish offset val
    offset(spmt)[ofst] = spmt.num_nzd;
    offset_row(spmt)[ofst - 1] = offset(spmt)[ofst] - offset(spmt)[ofst - 1];
    spmt.num_zd = num_col * num_row - spmt.num_nzd;
    return spmt;
}

/*  Function name:  vec2csr
    Discreption:    combination of vec2ele and ele2csr
    Parameters:
    Return:         
*/
spa_mat vec2csr(val_t *a, count_t num_row, count_t num_col)
{
    count_t k = 0;
    for (count_t i = 0; i < num_col * num_row; i++)
        k = a[i] ? k + 1 : k;
    ele *e = vec2ele(a, num_row, num_col);
    return ele2csr(e, num_row, num_col, k);
}

spa_mat mat2csr(val_t *a, count_t num_row, count_t num_col)
{

    count_t num_nzd = 0;
    /*count number of none zero data*/
    for (count_t i = 0; i < num_row * num_col; i++)
    {
        if (*(a + i))
            num_nzd += 1;
    }
    /*spmt init*/
    count_t ofst = 0;
    count_t last_nzd_row = 0;
    spa_mat spmt = {};
    spmt.num_row = num_row;
    spmt.num_col = num_col;
    spmt.val = calloc(num_nzd, sizeof(val_t));
    spmt.col_index = calloc(num_nzd, sizeof(index_t));
    spmt.offset = calloc(num_row + 1, sizeof(offset_t));
    spmt.offset_row = calloc(num_row, sizeof(offset_row_t));

    /* put into CSR */
    for (count_t i = 0; i < num_row; i++)
    {
        for (count_t j = 0; j < num_col; j++)
        {
            if (*(a + num_col * i + j) != 0)
            {
                if (i > last_nzd_row || !spmt.num_nzd && !last_nzd_row) //next row or first row
                {
                    offset(spmt)[ofst] = spmt.num_nzd;
                    if (ofst > 0)
                        offset_row(spmt)[ofst - 1] = offset(spmt)[ofst] - offset(spmt)[ofst - 1];
                    ofst += 1;
                }
                val(spmt)[spmt.num_nzd] = *(a + num_col * i + j);
                index(spmt)[spmt.num_nzd] = j;
                spmt.num_nzd += 1;
                last_nzd_row = i;
            }
            else
                spmt.num_zd += 1;
        }
    }
    //finish offset val
    offset(spmt)[ofst] = spmt.num_nzd;
    offset_row(spmt)[ofst - 1] = offset(spmt)[ofst] - offset(spmt)[ofst - 1];
    return spmt;
}

spa_mat mat2csr_partation(val_t *a, count_t src_row, count_t src_col, count_t dst_row, count_t dst_col, count_t index_row, count_t index_col)
{
    count_t start_row = index_row * dst_row;
    count_t start_col = index_col * dst_col;
    // printf("-----------------start pos:%d,%d-----------------\n", start_row, start_col);
    count_t num_nzd = 0;
    /*count number of none zero data*/
    for (count_t i = start_row; i < start_row + dst_row; i++)
        for (count_t j = start_col; j < start_col + dst_col; j++)
        {
            if (*(a + i * src_col + j))
            {
                num_nzd += 1;
                // printf("**%d, %d=%.2f\t", i, j, *(a + i * src_col + j));
            }
        }
    /*spmt init*/
    count_t ofst = 0;
    count_t last_nzd_row = 0;
    spa_mat spmt = {};
    spmt.num_row = dst_row;
    spmt.num_col = dst_col;
    spmt.val = calloc(num_nzd, sizeof(val_t));
    spmt.col_index = calloc(num_nzd, sizeof(index_t));
    spmt.offset = calloc(dst_row + 1, sizeof(offset_t));
    spmt.offset_row = calloc(dst_row, sizeof(offset_row_t));

    /* put into CSR */
    for (count_t i = start_row; i < start_row + dst_row; i++)
    {
        for (count_t j = start_col; j < start_col + dst_col; j++)
        {
            if (*(a + i * src_col + j) != 0)
            {
                if ((i - start_row) > last_nzd_row || !spmt.num_nzd && !last_nzd_row) //next row or first row
                {
                    offset(spmt)[ofst] = spmt.num_nzd;
                    if (ofst > 0)
                        offset_row(spmt)[ofst - 1] = offset(spmt)[ofst] - offset(spmt)[ofst - 1];
                    ofst += 1;
                }
                val(spmt)[spmt.num_nzd] = *(a + i * src_col + j);
                index(spmt)[spmt.num_nzd] = j - start_col;
                spmt.num_nzd += 1;
                last_nzd_row = i - start_row;
            }
            else
                spmt.num_zd += 1;
        }
    }
    //finish offset val
    offset(spmt)[ofst] = spmt.num_nzd;
    offset_row(spmt)[ofst - 1] = offset(spmt)[ofst] - offset(spmt)[ofst - 1];
    return spmt;
}

spa_mat *mat2csr_divide(val_t *a, count_t src_row, count_t src_col, count_t dst_row, count_t dst_col)
{
    size_t part_col = ceil(src_col / dst_col);
    size_t part_row = ceil(src_row / dst_row);
    //so can the index of partition be (part_row, part_col)
    spa_mat spmt[part_col * part_row];
    spa_mat spmt_tmp;

    // printf("partation number %d * %d\n", part_row, part_col);

    count_t start_col = 0;
    count_t start_row = 0;
    for (count_t i = 0; i < part_row; i++)
        for (count_t j = 0; j < part_col; j++)
        {
            if (src_col >= dst_col && src_row >= dst_row) //make sure dst less than src
            {
                if (i < part_row && j < part_col)
                {
                    // printf("\nworking on subset %d(%d, %d)\n", i * part_col + j, i, j);
                    spmt[i * part_col + j] = mat2csr_partation(a, src_row, src_col, dst_row, dst_col, i, j);
                    // printf("***num_nzd=%d", (spmt + i * part_col + j)->num_nzd);
                }
                else if (i == part_row && j < part_col)
                    ;
                else if (i < part_row && j == part_col)
                    ;
                else if (i == part_row && j == part_col)
                    ;
                //*spmt0.next = mat2csr_partation(a, src_row, src_col, dst_row, dst_col, i, j);
            }
        }
    // print_csr(&spmt_tmp);
    // puts("-----");

    print_csr(spmt);
    puts("-----");
    print_csr(spmt+1);
    puts("-----");
    print_csr(&spmt[2]);
    puts("-----");
    print_csr(&spmt[3]);
    puts("-----");
    return spmt;
}

//----------------------------------------DECOMPRESSION----------------------------------------//
/*  Function name:  csr2ele
    Discreption:    transfer csr style to none-zero elements.
    Parameters:
        @m          struct of sparse matrix.
    Return:         elements without zeros.
*/
ele *csr2ele(spa_mat *m)
{
    count_t row = 0;
    ele *e = calloc(m->num_row * m->num_col, sizeof(ele));
    for (count_t j = 0; j < m->num_nzd; j++)
    {
        row = j < offset(*m)[row + 1] ? row : row + 1;
        {
            val(e[j]) = val(*m)[j];
            col(e[j]) = index(*m)[j];
            row(e[j]) = row;
        }
    }
    return e;
}

/*  Function name:  ele2vec
    Discreption:    transfer none-zero data elements to sparse vectors.
    Parameters:     
        @vec        input elements.
        @num_row    row number of matrix like vector.
        @num_col    row number of matrix like vector.
    Return:         A sparse matrix stored in vector.
*/
val_t *ele2vec(ele *e, count_t num_row, count_t num_col)
{
    count_t k = 0;
    val_t *vec = calloc(num_col * num_row, sizeof(val_t));
    for (count_t i = 0; i < num_col * num_row; i++)
    {
        if (val(e[k]))
        {
            vec[num_col * row(e[k]) + col(e[k])] = val(e[k]);
            k++;
        }
    }
    return vec;
}

/*  Function name:  csr2vec
    Discreption:    combination of csr2ele and ele2vec
    Parameters:
    Return:         
*/
val_t *csr2vec(spa_mat *m)
{
    return ele2vec(csr2ele(m), m->num_row, m->num_col);
}

val_t *csr2mat(spa_mat *m)
{
    val_t *mat = calloc(m->num_col * m->num_row, sizeof(val_t));
    count_t row_idx = 0;

    for (count_t k = 0; k < m->num_nzd; k++)
    {
        row_idx = k < m->offset[row_idx + 1] ? row_idx : row_idx + 1;
        mat[m->num_col * row_idx + m->col_index[k]] = m->val[k];
    }
    return mat;
}

val_t *csr2mat_new(spa_mat *m)
{
    val_t *mat = calloc(m->num_col * m->num_row, sizeof(val_t));
    count_t row_idx = 0;
    count_t row_val_used = 0;
    for (count_t k = 0; k < m->num_nzd; k++)
    {

        if (row_val_used < m->offset_row[row_idx])
            row_val_used += 1;
        else
        {
            row_idx += 1;
            row_val_used = 1;
        }
        mat[m->num_col * row_idx + m->col_index[k]] = m->val[k];
    }
    return mat;
}

//----------------------------------------PRINT_FUNCTION----------------------------------------//
/*  Function name:  print_compress_sparse
    Discreption:    print csr
    Parameters:
        @m          struct of sparse matrix.
    Return:         null
*/
void print_csr(spa_mat *m)
{
    printf("num_nzd = %ld, num_zd = %ld, num_col = %ld, num_ row = %ld\n",
           m->num_nzd, m->num_zd, m->num_col, m->num_row);
    printf("value =");
    for (unsigned long i = 0; i < m->num_nzd; i++)
    {
        printf(" %f", val(*m)[i]);
    }
    printf("\n");

    printf("index =");
    for (unsigned long i = 0; i < m->num_nzd; i++)
        printf(" %lu", index(*m)[i]);
    printf("\n");

    printf("ofset =");
    for (unsigned long i = 0; i < m->num_row + 1; i++)
        printf(" %lu", offset(*m)[i]);
    printf("\n");

    printf("ofstr =");
    for (unsigned long i = 0; i < m->num_row; i++)
        printf(" %lu", offset_row(*m)[i]);
    printf("\n");
}

/*  Function name:  print_ele
    Discreption:    print elements
    Parameters:
        @a          vector
        @num_row    row number of matrix
        @num_col    col number of matrix
    Return:         null
    Note:           honestly it's useless unless when debug
    FIXME:          parameters should have a indicate of size of elements, or I can calculate with sizeof(.)
*/
void print_ele(ele *e, count_t num_row, count_t num_col)
{
    for (count_t i = 0; i < 12; i++)
    {
        printf("element %d, val = %f, pos = %d,%d\n", i, val(e[i]), row(e[i]), col(e[i]));
    }
}

/*  Function name:  print_vec
    Discreption:    print vector as a matrix
    Parameters:
        @a          vector
        @num_row    row number of matrix
        @num_col    col number of matrix
    Return:         null
*/
void print_vec(val_t *a, count_t num_row, count_t num_col)
{
    for (count_t i = 0; i < num_row * num_col; i++)
    {
        if (i % num_col)
            printf("%.1f  ", a[i]);
        else
            printf("\n%.1f  ", a[i]);
    }
}
//----------------------------------------TEST----------------------------------------//
#ifndef CSR
int main(int argc, char const *argv[])
{
    puts("----testing sparse matrix----");
    float mat_allz[] = {
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0};
    float mat_smal[] = {
        1, 7, 0, 0,
        0, 2, 8, 0,
        5, 0, 3, 9,
        0, 6, 0, 4};
    float mat_full[6][6] = {
        1, 7, 9, 9, 9, 9,
        9, 2, 8, 9, 9, 5,
        5, 9, 3, 9, 9, 6,
        9, 6, 9, 4, 9, 9,
        9, 9, 9, 9, 1, 1,
        1, 9, 1, 9, 6, 6};
    float mat_norm[6][6] = {//*****
                            1, 7, 0, 0, 0, 0,
                            0, 2, 8, 0, 0, 5,
                            5, 0, 3, 9, 0, 6,
                            0, 6, 0, 4, 0, 0,
                            0, 0, 0, 0, 0, 0,
                            1, 0, 1, 0, 0, 6};
    // number of ele
    count_t num_row = 6, num_col = 6;
    //count_t num_row = 4, num_col = 4;

    // ele,index -> num of ele; offset -> number of row
    print_vec(mat_norm, num_row, num_col);
    puts("\n---------------------------------");

    // spa_mat spmat = vec2csr(mat_norm, num_row, num_col);
    // print_csr(&spmat);
    // puts("\n---------------------------------");

    // // ele *e = vec2ele(mat_norm, num_row, num_col);
    // // print_ele(e, num_row, num_col);
    // // puts("\n---------------------------------");

    // float *c = csr2vec(&spmat);
    // float *d = csr2mat(&spmat);
    // float *e = csr2mat_new(&spmat);

    // print_vec(c, num_row, num_col);
    // puts("\n---------------------------------");

    // print_vec(e, num_row, num_col);
    // puts("\n---------------------------------");

    // float *c = calloc(num_col * num_row, sizeof(val_t));

    // puts("---------------");
    // print_vec(csr2vec(&spmat), 6, 6);
    // puts("\n---------------");
    spa_mat *spmt_arry = mat2csr_divide(mat_norm, num_row, num_col, 3, 3);
    //print_csr(*spmt_arry[0]);
    puts("\n---------------");
    spa_mat spmt_norm = mat2csr(mat_norm, num_row, num_col);
    print_csr(&spmt_norm);
    puts("\n---------------");
    spa_mat spmt_norm2 = vec2csr(mat_norm, num_row, num_col);
    print_csr(&spmt_norm2);
    // puts("\n---------------");
    // spa_mat spmt_full = mat2csr(mat_full, num_row, num_col);
    // print_csr(&spmt_full);
    // spa_mat spmt_full2 = vec2csr(mat_full, num_row, num_col);
    // print_csr(&spmt_full2);
    // puts("\n---------------");
    // spa_mat spmt_allz = mat2csr(mat_allz, num_row, num_col);
    // print_csr(&spmt_allz);
    // spa_mat spmt_allz2 = vec2csr(mat_allz, num_row, num_col);
    // print_csr(&spmt_allz2);
    // puts("\n---------------");
    // num_row = 4, num_col = 4;
    // spa_mat spmt_smal = mat2csr(mat_smal, num_row, num_col);
    // print_csr(&spmt_smal);
    // spa_mat spmt_smal2 = vec2csr(mat_smal, num_row, num_col);
    // print_csr(&spmt_smal2);
    // puts("\n---------------");
    // puts("\noriginal sparse");
    // print_vec(b, num_row, num_col);

    // puts("\nafter vec2ele");
    // ele *ele_test = vec2ele(a, num_row, num_col);
    // print_ele(ele_test, num_row, num_col);
    // spa_mat spmat_test = ele2csr(ele_test, num_row, num_col);

    // puts("\nafter ele2vec");
    // print_vec(ele2vec(ele_test, num_row, num_col), num_row, num_col);

    // free(ele_test);

    // puts("\nafter compress");
    // print_csr(&spmat_test);

    // puts("\nafter decompress");
    // ele *ele_decomp = csr2ele(&spmat_test);
    // print_ele(ele_decomp, num_row, num_col);

    // float *c = ele2vec(ele_decomp, num_row, num_col);
    // puts("\nafter ele2vec");
    // print_vec(c, num_row, num_col);

    // float *d = csr2vec(&spmat_test);
    // print_vec(d, num_row, num_col);

    return 0;
}
#endif