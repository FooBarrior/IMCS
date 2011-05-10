
public class Matrix{
	private double arr[][];

	public static class EMatrix extends Exception{};
	public static class EInvalidMatrix extends EMatrix{};
	public static class ECalculationImpossible extends EMatrix{};
	public Matrix(int h, int w){
		arr = new double[h][w];
	}
	public Matrix(int w){
		this(w, w);
	}

	public Matrix(Matrix proto){
		arr = proto.arrayCopy();
	}

	public int height(){
		return arr.length;
	}
	public int width(){
		return arr[0].length;
	}

	public double getElement(int i, int j){
		return arr[i][j];
	}
	public void setElement(int i, int j, double element){
		arr[i][j] = element;
	}
	
	static private double[][] arrCopyStrikedOut(double[][] a, int row, int col){
		int rLen = a.length, cLen = a[0].length;
		if(row >= 0) rLen--;
		if(col >= 0) cLen--;
		double[][] res = new double[rLen][cLen];
		//System.out.println("row: " + row + " col: "+ col);
		for(int i = 0, r = 0; i < a.length; r++, i++)
			if(i != row) for(int j = 0, c = 0; j < a[0].length; c++, j++){
				if(j != col){
					res[r][c] = a[i][j];
				}
				else c--;
			}	
			else r--;
		return res;
	}
	static private double[][] arrCopy(double[][] a){
		return arrCopyStrikedOut(a, -1, -1);
	}

	public double[][] arrayCopy(){
		return arrCopy(arr);
	}

	public Matrix transponented(){
		Matrix t = new Matrix(width(), height());
		for(int i = 0; i < width(); i++)
			for(int j = 0; j < height(); j++)
				t.setElement(i, j, arr[j][i]);
		return t;
	}

	private double determ(double[][] a){
		if(a.length == 1) return a[0][0];

		double res = 0, sign = 1;
		for(int i = 0; i < a[0].length; i++){
			res += a[0][i] * determ(arrCopyStrikedOut(a, 0, i)) * sign;
			sign = -sign;
		}
		return res;
	}

	public double det() throws EInvalidMatrix{
		if(height() != width()) throw new EInvalidMatrix();
		return determ(arrCopy(arr));
	}

	public Matrix invertible() throws EInvalidMatrix{
		double d = det();
		if(d == 0) throw new EInvalidMatrix();
		
		Matrix inv = new Matrix(height());
		for(int i = 0; i < height(); i++)
			for(int j = 0; j < width(); j++)
				inv.setElement(i, j, determ(arrCopyStrikedOut(arr, i, j)) / d);
		return inv;
	}

	public Matrix add(Matrix m) throws ECalculationImpossible{
		if(m.height() != height() || m.width() != width()) throw new ECalculationImpossible();

		for(int i = 0; i < height(); i++)
			for(int j = 0; j < width(); j++)
				m.setElement(i, j, m.getElement(i, j) + arr[i][j]);
		return this;
	}

	static public Matrix sum(Matrix l, Matrix r) throws ECalculationImpossible{
		return (new Matrix(l)).add(r);
	}

	static public Matrix multiply(Matrix l, Matrix r) throws ECalculationImpossible{
		if(l.width() != r.height()) throw new ECalculationImpossible();

		Matrix res = new Matrix(l.height(), r.width());
		for(int i = 0; i < l.height(); i++)
			for(int j = 0; j < r.width(); j++)
				for(int k = 0; k < l.width(); k++)
					res.setElement(i, j, l.getElement(i, k) * r.getElement(k, j));
		return res;
	}

};
