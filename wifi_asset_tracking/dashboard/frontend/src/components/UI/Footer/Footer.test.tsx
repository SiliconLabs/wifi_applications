import { render, screen } from '@testing-library/react';
import '@testing-library/jest-dom';
import Footer from './Footer';

describe('Footer Component', () => {
  test('renders footer with correct text', () => {
    render(<Footer />);
    const footerText = screen.getByText('Copyright © 2024 Silicon Laboratories. All rights reserved.');
    expect(footerText).toBeInTheDocument();
  });
});
