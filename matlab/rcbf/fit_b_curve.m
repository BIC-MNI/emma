function err = fit_b_curve (args, shifted_g_even, ts_even, A, fstart, flengths)

integral = b_curve (args, shifted_g_even, ts_even, A, fstart, flengths);

err = sum((A - integral).^2);
%err = A - integral;
