
#include <type_traits>
#include <array>
#include <iostream>

//index_tuple<int... N>
//intの列を格納するための型
//配列の添字の列を意味する
template<int... Ns>
struct index_tuple{};

//add_tuple<Tuple, Elem>
//index_tupleの最後にElemを追加した新しいindex_tupleを生成するメタ関数
//
//ex) add_tuple<index_tuple<1,2>,3>::type
//    ==>> index_tuple<1,2,3>
template<typename Tuple, int Elem>
struct add_tuple;

template<int... Elems, int NewElem>
struct add_tuple<index_tuple<Elems...>, NewElem>
{
	typedef index_tuple<Elems..., NewElem> type;
};

//index_range<int N>
//index_tuple<0,1,2,..,N-1> を作るメタ関数
template<int N>
struct index_range
{
	typedef typename add_tuple<typename index_range<N-1>::type, N-1>::type type;
};

template<>
struct index_range<0>
{
	typedef index_tuple<> type;
};

//concatenate<typename Tuple1, int Elem, typename Tuple2>
//index_tupleを2つと、値1個をくっつけて新しいindex_tupleを作るメタ関数
//
//ex) concatenate<index_tuple<1,2>,3,index_tuple<4,5,6>>::type
//    ==>> index_tuple<1,2,3,4,5,6>
template<typename Tuple1, int Elem, typename Tuple2>
struct concatenate;

template<int... Elems1, int ElemMid, int... Elems2>
struct concatenate<index_tuple<Elems1...>, ElemMid, index_tuple<Elems2...>>
{
	typedef index_tuple<Elems1..., ElemMid, Elems2...> type;
};

//pivoting<const int* Array, int Pivot, typename Smaller, typename Larger, typename Rest>
//クイックソートの本体部分
//Rest（Arrayの添字）を順に、Arrayの値をPivotと比較して、小さい方をSmaller、大きい方をLargerに振り分ける（＝はLarger）
//振り分け終わったらSmallerとLargerをまたクイックソートしてから、くっつけて1つのindex_tupleにする
//
//quick_sort_impl<const int* Array, Tuple>
//Tupleの先頭をPivotにしてpivotingに投げる
//Tupleが空（N==0）なら空のindex_tupleを返す
//
template<const int* Array, int Pivot, typename Smaller, typename Larger, typename Rest>
struct pivoting;

template<const int* Array, typename Tuple>
struct quick_sort_impl;

template<const int* Array, int ElemTop, int... ElemsRest>
struct quick_sort_impl<Array, index_tuple<ElemTop, ElemsRest...>>
{
	typedef typename pivoting<
		Array, 
		ElemTop, 
		index_tuple<>, 
		index_tuple<>, 
		index_tuple<ElemsRest...>
	>::type type;
};

template<const int* Array>
struct quick_sort_impl<Array, index_tuple<>>
{
	typedef index_tuple<> type;
};

template<const int* Array, int Pivot, int... ElemsS, int... ElemsL, int ElemRTop, int... ElemsRRest>
struct pivoting<Array, Pivot, index_tuple<ElemsS...>, index_tuple<ElemsL...>, index_tuple<ElemRTop,ElemsRRest...>>
{
	typedef typename pivoting<Array, Pivot, 
		typename std::conditional<
			(Array[ElemRTop] < Array[Pivot]),
			index_tuple<ElemsS..., ElemRTop>, 
			index_tuple<ElemsS...>
		>::type,
		typename std::conditional<
			(Array[ElemRTop] < Array[Pivot]),
			index_tuple<ElemsL...>, 
			index_tuple<ElemsL..., ElemRTop>
		>::type,
		index_tuple<ElemsRRest...>
	>::type type;
};

template<const int* Array, int Pivot, int... ElemS, int... ElemL>
struct pivoting<Array, Pivot, index_tuple<ElemS...>, index_tuple<ElemL...>, index_tuple<>>
{
	typedef typename concatenate<
		typename quick_sort_impl<Array, index_tuple<ElemS...>>::type,
		Pivot,
		typename quick_sort_impl<Array, index_tuple<ElemL...>>::type
	>::type type;
};

//arranged_array<const int *Array, typename Tuple>
//ArrayをTupleの添字の順番に並べ替えた配列を抱えるクラス
//
template<const int *Array, typename Tuple>
struct arranged_array;

template<const int *Array, int... Elems>
struct arranged_array<Array, index_tuple<Elems...>>
{
//	static constexpr int sorted[sizeof...(Elems)] = { Array[Elems]... };
	int sorted[sizeof...(Elems)];
	constexpr arranged_array() : sorted{ Array[Elems]... } {};
};

//quick_sort<const int* Array, N>
//クイックソートのインターフェース
//sortedメンバがソート済み配列
//
template<const int *Array, int N>
struct quick_sort : public arranged_array<Array, typename quick_sort_impl<Array, typename index_range<N>::type>::type >
{};


constexpr int hoge_ar[] = {3,1,4,5,2};

int main()
{
	static_assert(std::is_same<quick_sort_impl<hoge_ar, index_range<5>::type>::type, index_tuple<1,4,0,2,3> >::value, "Not same type");
	
	const int* sorted = quick_sort<hoge_ar,5>().sorted;
	for(int i=0; i<5; i++){
		std::cout << sorted[i] << std::endl;
	}
}
