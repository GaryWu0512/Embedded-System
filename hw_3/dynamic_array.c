#include "dynamic_array.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "arbitrary_array.h"

/* private functions *********************************************************/
static int array_num = 0;
//static int t = 0;
//static DynamicArray* arg[200];

ArbitraryArray *AA = ArbitraryArray_new(100);

/*static void arrang_size (){
    DynamicArray* arg[array_num+10]; 
}*/
/*void delect_arr (DynamicArray * da){
    int i, delectIndex;
    DynamicArray * delect;
    for (i = 0 ; i<array_num; i++){
        if(delect == arg[i]) {
            delectIndex=i;
        }
    }
    for(i=delectIndex ; i<array_num-1 ; i++){
        arg[i]=arg[i+1];
    }
    //array_num = array_num-1;
    //printf("array_num = %d", array_num);
}*/

/* Position in the buffer of the array element at position index */
static int index_to_offset ( const DynamicArray * da, int index ) {
    return index + da->origin;
}

/* Position of the element at buffer position 'offset' */
static int offset_to_index ( const DynamicArray * da, int offset ) {
    return offset - da->origin;
}

/* Non-zero if and only if offset lies ouside the buffer */
static int out_of_buffer ( DynamicArray * da, int offset ) {
    return offset < 0 || offset >= da->capacity;
}

/* Makes a new buffer that is twice the size of the old buffer,
   copies the old information into the new buffer, and deletes
   the old buffer */
static void extend_buffer ( DynamicArray * da ) {

    double * temp = (double *) calloc ( 2 * da->capacity, sizeof(double) );
    int new_origin = da->capacity - (da->end - da->origin)/2,
           new_end = new_origin + (da->end - da->origin);

    for ( int i=0; i<DynamicArray_size(da); i++ ) {
        temp[new_origin+i] = DynamicArray_get(da,i);
    }

    free(da->buffer);
    da->buffer = temp;

    da->capacity = 2 * da->capacity;
    da->origin = new_origin;
    da->end = new_end;

    return;

}

/* public functions **********************************************************/

DynamicArray * DynamicArray_new(void) {
    DynamicArray * da = (DynamicArray *) malloc(sizeof(DynamicArray));
    da->capacity = DYNAMIC_ARRAY_INITIAL_CAPACITY;    
    da->buffer = (double *) calloc ( da->capacity, sizeof(double) ); 
    da->origin = da->capacity / 2;
    da->end = da->origin;
    ArbitraryArray_set_from_ptr(AA, array_num, &da);
    //t =t+1;
    array_num++;
    return da;
}

void DynamicArray_destroy(DynamicArray * da) {
    int i;
    for (i = 0; i<array_num ; i++){
        DynamicArray ** b = (DynamicArray **) ArbitraryArray_get_ptr(AA,i);
        if ( (*b) == da){
            for(; i<array_num -1 ; i++ ){
                DynamicArray ** c = (DynamicArray **) ArbitraryArray_get_ptr(AA,i+1);
                ArbitraryArray_set_from_ptr(AA, i, c);
            }
        }
    }

    free(da->buffer);
    da->buffer = NULL;
    array_num--;
    return;
}

int DynamicArray_size(const DynamicArray * da) {
    assert(da->buffer != NULL);
    return da->end - da->origin;
}

char * DynamicArray_to_string(const DynamicArray * da) {
    assert(da->buffer != NULL);
    char * str = (char *) calloc (20,DynamicArray_size(da)),
         temp[20];
    int j = 1;
    str[0] = '[';
    for ( int i=0; i < DynamicArray_size(da); i++ ) {
        if ( DynamicArray_get(da,i) == 0 ) {
            snprintf ( temp, 20, "0" );
        } else {
            snprintf ( temp, 20, "%.5lf", DynamicArray_get(da,i) ); 
        }
        if ( i < DynamicArray_size(da) - 1 ) {
            sprintf( str + j, "%s,", temp);
            j += strlen(temp) + 1;
        } else {
            sprintf( str + j, "%s", temp);
            j += strlen(temp);
        }

    }
    str[j] = ']';
    return str;
}

void DynamicArray_print_debug_info(const DynamicArray * da) {

    char * s = DynamicArray_to_string(da);
    printf ( "  %s\n", s);
    printf ( "  capacity: %d\n  origin: %d\n  end: %d\n  size: %d\n\n",
      da->capacity, 
      da->origin, 
      da->end,
      DynamicArray_size(da));

    free(s);

}

void DynamicArray_set(DynamicArray * da, int index, double value) {
    assert(da->buffer != NULL);
    assert ( index >= 0 );
    while ( out_of_buffer(da, index_to_offset(da, index) ) ) {
        extend_buffer(da);
    }
    da->buffer[index_to_offset(da, index)] = value;
    if ( index >= DynamicArray_size(da) ) {
        da->end = index_to_offset(da,index+1);
    }

}

double DynamicArray_get(const DynamicArray * da, int index) {
    assert(da->buffer != NULL);
    assert ( index >= 0 );
    if ( index >= DynamicArray_size(da) ) {
        return 0;
    } else {
        return da->buffer[index_to_offset(da,index)];
    }
}

void DynamicArray_push(DynamicArray * da, double value ) {
    DynamicArray_set(da, DynamicArray_size(da), value );
}

void DynamicArray_push_front(DynamicArray * da, double value) {
    assert(da->buffer != NULL);
    while ( da->origin == 0 ) {
        extend_buffer(da);
    }
    da->origin--;
    DynamicArray_set(da,0,value);
}

double DynamicArray_pop(DynamicArray * da) {
    assert(DynamicArray_size(da) > 0);
    double value = DynamicArray_last(da);
    DynamicArray_set(da, DynamicArray_size(da)-1, 0.0);
    da->end--;
    return value;
}

double DynamicArray_pop_front(DynamicArray * da) {
    assert(DynamicArray_size(da) > 0);
    double value = DynamicArray_first(da);
    da->origin++;
    return value;    
}

DynamicArray * DynamicArray_map(const DynamicArray * da, double (*f) (double)) {
    DynamicArray * result = DynamicArray_copy(da);
    for ( int i=0; i<DynamicArray_size(da); i++ ) {
        DynamicArray_set(result, i, f(DynamicArray_get(da, i)));
    }
    return result;
}

double DynamicArray_min(const DynamicArray * da){
    assert(DynamicArray_size(da) > 0);
    double min  = da->buffer[index_to_offset(da, 0)];
    int i;
    for (i = 1; i<DynamicArray_size(da); i++){
        if ( da->buffer[index_to_offset(da, i)] < min ){
            min = da->buffer[index_to_offset(da, i)];
        }
    }
    return min;
}

double DynamicArray_max(const DynamicArray * da){
    assert(DynamicArray_size(da) > 0);
    double max  = da->buffer[index_to_offset(da, 0)];
    int i;
    for (i = 1; i<DynamicArray_size(da); i++){
        if ( da->buffer[index_to_offset(da, i)] > max ){
            max = da->buffer[index_to_offset(da, i)];
        }
    }
    return max;
}

double DynamicArray_mean(const DynamicArray * da){
    assert(DynamicArray_size(da) > 0);
    double b, mean, total = 0;
    int i ;
    for (i = 0; i<DynamicArray_size(da); i++){
        b = da->buffer[index_to_offset(da, i)];
        total = total + b;
    }
    mean = total / DynamicArray_size(da);
    return mean;
}

double DynamicArray_median(const DynamicArray * da){
    assert(DynamicArray_size(da) > 0);
    double median, temp;
    double temp_arr[DynamicArray_size(da)];
    int i,j,k;
    i = DynamicArray_size(da)/2;

    for( j = 0; j < DynamicArray_size(da); j++) {
        for( k = j; k < DynamicArray_size(da); k++) {
            if( da->buffer[index_to_offset(da, j)] < da->buffer[index_to_offset(da, k)] ) {
                temp = da->buffer[index_to_offset(da, j)];
                temp_arr[j] = da->buffer[index_to_offset(da, k)];
                temp_arr[k] = temp;
            }
        }
    }

    if ( DynamicArray_size(da)%2 == 0 ){
        median = (temp_arr[i-1]+temp_arr[i])/2;
    }else{
        median = temp_arr[i];
    }
    return median;
}

double DynamicArray_sum(const DynamicArray * da){
    //assert(DynamicArray_size(da) > 0);
    if (DynamicArray_size(da) == 0){
        return 0.0;
    }
    double b, total = 0;
    int i;
    for (i = 0; i<DynamicArray_size(da); i++){
        b = da->buffer[index_to_offset(da, i)];
        total = total + b;
        //printf("total = %f", total);
    }
    return total;
}

double DynamicArray_last(const DynamicArray * da){
    assert(DynamicArray_size(da) > 0);
    double last;
    last = DynamicArray_get(da, DynamicArray_size(da)-1);
    return last;
}

double DynamicArray_first(const DynamicArray * da){
    assert(DynamicArray_size(da) > 0);
    double first;
    first = DynamicArray_get(da, 0);
    return first;
}

DynamicArray * DynamicArray_copy(const DynamicArray * da) {
    DynamicArray * db = (DynamicArray *) malloc(sizeof(DynamicArray));
    db->capacity = DYNAMIC_ARRAY_INITIAL_CAPACITY;    
    db->buffer = (double *) calloc ( db->capacity, sizeof(double) ); 
    db->origin = da->origin;
    db->end = db->origin;
    int i;
    for (i = 0; i<DynamicArray_size(da); i++){
      DynamicArray_push(db, DynamicArray_get(da, i));
    }
    ArbitraryArray_set_from_ptr(AA, array_num, &db);
    //arg[array_num] = db;
    //t = t+1;
    array_num++;
    return db;
}

DynamicArray * DynamicArray_range(double c, double d, double step){
    DynamicArray * a = DynamicArray_new();
    if (abs(step)>0){
        int i;
        int times = ((d-c)/step)+1;
        for (i = 0; i < times ; i++){
            DynamicArray_push(a,c+(i*step));
        }
        return a;
    }else{
        return a;
    }
}

DynamicArray * DynamicArray_concat ( const DynamicArray * a, const DynamicArray * b ){
    DynamicArray * c = DynamicArray_copy(a);
    int j;
    double nm_b;
    for (j = 0;j<DynamicArray_size(b);j++){
        nm_b = b->buffer[index_to_offset(b, j)];
        DynamicArray_push(c,nm_b);
    }
    return c;
}

DynamicArray * DynamicArray_take(const DynamicArray * a, double b){
    int space = abs(b);
    DynamicArray *s = DynamicArray_new();
    int i;
    double nm;
    if (b>0){
        for (i = 0; i<space; i++){
            if (a->buffer[index_to_offset(a, i)] != 0 ){
                nm = DynamicArray_get(a, i);
                DynamicArray_push(s,nm);
            }else{
                DynamicArray_push(s,0);
            }            
        }
    }else{
        for (i = 0; i<space; i++){
            if (a->buffer[index_to_offset(a, DynamicArray_size(a)- space+i)] != 0 ){
                nm = DynamicArray_get(a, DynamicArray_size(a)- space+i);
                DynamicArray_push(s,nm);
            }else{
                DynamicArray_push(s,0);
            }            
        }
    }
    return s;
}

int DynamicArray_num_arrays(){
    return array_num;
}

int DynamicArray_is_valid(const DynamicArray * da){
    if (da->buffer != NULL) {
        return 1;
    }else{
        return 0;
    }
}

int DynamicArray_destroy_all(){
    int i;;
    int j = array_num;
    //arrang_size ();
    //printf("the size is ", sizeof(*arg));
    //printf("nm === %d", array_num);
    for(i = 0; i<j; i++){
        DynamicArray ** b = (DynamicArray **) ArbitraryArray_get_ptr(AA,i);
        free((*b)->buffer);
        (*b)->buffer = NULL;
        //printf("destory 1");
        array_num--;
        //printf("nm === %d", array_num);     
    }
    array_num = 0;
    //t = 0;
    return array_num;
}
