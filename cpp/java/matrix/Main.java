
public class Main{
	static void outp(Matrix m){
		for(int i = 0; i < m.height(); i++){
			for(int j = 0; j < m.width(); j++)
				System.out.print(m.getElement(i, j) + "\t");
			System.out.println();
		}
	}
	public static void main(String[] args){
		Matrix mx = new Matrix(5, 5);
		double xs[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
		for(int i = 0; i < mx.height(); i++)
			for(int j = 0, x = 1; j < mx.width(); j++){
				mx.setElement(i, j, x);
				x *= xs[i];
			}
		double d = 1;
		for(int i = 0; i < mx.height(); i++)
			for(int j = 0; j < i; j++)
				d *= (xs[j] - xs[i]);
		System.out.println(d);
		outp(mx);
		try{
			System.out.println("eq? " + (d == mx.det()));
			outp(Matrix.multiply(mx, mx.invertible()));
		} catch(Matrix.EMatrix e){
			System.out.println("badbadbad");
		}
	}
};
