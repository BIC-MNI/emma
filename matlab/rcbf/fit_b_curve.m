function err = fit_b_curve (args)

global A

integral = b_curve (args);

err = sum((A - integral).^2);
