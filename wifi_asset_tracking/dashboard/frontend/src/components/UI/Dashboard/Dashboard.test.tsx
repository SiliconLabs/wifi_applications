import { render, screen } from '@testing-library/react';
import '@testing-library/jest-dom';
import { describe, it, vi, expect } from 'vitest';
import Dashboard from './Dashboard';

// Mock the Header, Footer, Content components and the useNavigationPrompt hook
vi.mock('../Header/Header', () => {
  return {
    default: function MockHeader() {
      return <div>Header</div>;
    },
  };
});

vi.mock('../Footer/Footer', () => {
  return {
    default: function MockFooter() {
      return <div>Footer</div>;
    },
  };
});

vi.mock('../Content/Content', () => {
  return {
    default: function MockContent() {
      return <div>Content</div>;
    },
  };
});

vi.mock('../../../hooks/useNavigationPrompt', () => {
  return {
    default: vi.fn(),
  };
});

describe('Dashboard Component', () => {
  it('renders the Dashboard component with Header, Content, and Footer', () => {
    render(<Dashboard />);
    // Check if the Header is rendered
    expect(screen.getByText('Header')).toBeInTheDocument();

    // Check if the Content is rendered
    expect(screen.getByText('Content')).toBeInTheDocument();

    // Check if the Footer is rendered
    expect(screen.getByText('Footer')).toBeInTheDocument();
  });
});
