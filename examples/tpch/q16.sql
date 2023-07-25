part
.filter(p_brand = 'Brand#45' && !(p_type.like('MEDIUM POLISHED%')) && p_size.in(49, 14, 23, 45, 19, 3, 36, 9))
.join(partsupp, p_partkey = ps_partkey)
.join(supplier.filter(s_comment.like('%Customer%Complaints%')), s_suppkey = ps_suppkey, LeftAnti)
.groupby({p_brand, p_type, p_size}, {supplier_cnt:=count()})
.orderby({supplier_cnt.desc(), p_brand, p_type, p_size})
