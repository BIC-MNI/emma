function err = fit_b_curve (args, shifted_g_even, ts_even, A, midftimes)

integral = b_curve (args, shifted_g_even, ts_even, A, midftimes);

err = sum((A - integral).^2);
% err = A - integral;
